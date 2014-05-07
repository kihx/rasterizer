#include "stdafx.h"
#include "concommand.h"
#include "mathlib.h"
#include "memory.h"
#include "system.h"
#include "threading.h"


#define MEMORY_THREAD_SAFE


namespace kih
{
	/* Global allocator accessors
	*/
	using AllocableGetter = std::function<IAllocable*()>;

	static AllocableGetter s_GlobalAllocableGetter;
	static AllocableGetter s_ThreadLocalAllocableGetter;

	static void InstallAllocableGetter( AllocableGetter func )
	{
		s_GlobalAllocableGetter = func;
	}

	static void InstallThreadLocalAllocatorGetter( AllocableGetter func )
	{
		s_ThreadLocalAllocableGetter = func;
	}

	IAllocable* GetGlobalAllocator()
	{
		return s_GlobalAllocableGetter ? s_GlobalAllocableGetter() : nullptr;
	}

	static IAllocable* GetThreadLocalAllocator()
	{
		return s_ThreadLocalAllocableGetter ? s_ThreadLocalAllocableGetter() : nullptr;
	}

	IAllocable* GetCurrentAllocator()
	{
#ifdef MEMORY_THREAD_SAFE
		if ( Thread::IsInMainThread() )
		{
			return GetGlobalAllocator();
		}
		else
		{
			return GetThreadLocalAllocator();
		}
#else
		return GetGlobalAllocator();
#endif // MEMORY_THREAD_SAFE
	}


	/* class ArrayMT: thread-safe simple and fast dynamic array
	*/
	template<class T>
	class ArrayMT
	{
		NONCOPYABLE_CLASS( ArrayMT );

	public:
		ArrayMT() :
			m_elements( nullptr ),
			m_size( 0 ),
			m_capacity( 0 )
		{
		}

		~ArrayMT()
		{
			Purge();
		}

		FORCEINLINE size_t Capacity() const
		{
			return m_capacity;
		}

		FORCEINLINE void Reserve( size_t capacity )
		{
			LockGuard<SpinLock> guard( m_lock );

			ReallocNonConstruct( Max( capacity, 1 ) );
		}

		FORCEINLINE size_t Size() const
		{
			return m_size;
		}

		FORCEINLINE void Add( const T& elem )
		{
			LockGuard<SpinLock> guard( m_lock );

			CheckIncrement();
			m_elements[m_size++] = elem;
		}

		void Remove( size_t index )
		{
			assert( index + 1 <= m_size );

			LockGuard<SpinLock> guard( m_lock );

			--m_size;
			if ( index == m_size )
			{
				return;
			}

			// Shift elements to left.
			for ( size_t i = index; i < m_size; ++i )
			{
				m_elements[i] = m_elements[i + 1];
			}
		}

		void RemoveAll()
		{
			m_size = 0;
		}

		void Purge()
		{
			LockGuard<SpinLock> guard( m_lock );

			::free( m_elements );
			m_elements = nullptr;
			m_size = 0;
			m_capacity = 0;
		}

		FORCEINLINE const T& operator[]( size_t index ) const
		{
			assert( index >= 0 && index < Size() );
			return m_elements[index];
		}

		FORCEINLINE T& operator[]( size_t index )
		{
			assert( index >= 0 && index < Size() );
			return m_elements[index];
		}

	private:
		void ReallocNonConstruct( size_t capacity )
		{
			size_t copyableSize = Min( Size(), capacity );
			T* newElements = static_cast<T*>( ::malloc( sizeof( T ) * capacity ) );
			if ( m_elements )
			{
				for ( size_t i = 0; i < copyableSize; ++i )
				{
					newElements[i] = m_elements[i];
				}
			}
			m_elements = newElements;
			m_size = copyableSize;
			m_capacity = capacity;
		}

		// Checks a free space to add one element and expands if neccesary
		FORCEINLINE void CheckIncrement()
		{
			if ( m_size + 1 > m_capacity )
			{
				ReallocNonConstruct( m_capacity + m_capacity / 2 + 1 );
			}
		}

	private:
		T* m_elements;
		size_t m_capacity;
		size_t m_size;
		SpinLock m_lock;
	};


	/* Low Fragmentation Heap (LFH)
	*/
	struct LFHBlock final
	{
		LFHBlock* Next;		// next block pointer
		byte* Address;		// heap address
#ifdef DEBUG_ALLOCATION
		const char* FileName;
		int Line;
		size_t Index;
		LFHBlock* Prev;
		bool Used;
#endif
	};

	void LFHBlock_MakeNull( LFHBlock& block )
	{
#ifdef DEBUG_ALLOCATION
		block.FileName = "";
		block.Line = -1;
		block.Index = 0;
		block.Prev = nullptr;
		block.Used = false;
#endif	
		block.Address = nullptr;
		block.Next = nullptr;
	}

	class LFHBucket final
	{
	public:
		LFHBucket();
		~LFHBucket();

		FORCEINLINE size_t BlockSize() const
		{
			return m_blockSize;
		}

		FORCEINLINE size_t BlockCount() const
		{
			return m_blockCount;
		}

		FORCEINLINE size_t UsedBlockCount() const
		{
			return m_usedBlockCount;
		}

		// Returns reserved heap size of this bucket in bytes
		FORCEINLINE size_t ReservedBytes() const
		{
			return m_blockSize * m_blockCount;
		}

		// Returns committed heap size of this bucket in bytes
		FORCEINLINE size_t CommittedBytes() const
		{
			return m_committedBytes;
		}

		FORCEINLINE size_t AllocatedBytes() const
		{
			return BlockCount() * UsedBlockCount();
		}

		FORCEINLINE bool IsFull() const
		{
			return m_freeBlockHead == nullptr;
		}

		void PrintMemoryUsage() const;

		// only call by LFHAllocator
	private:
		friend class LFHAllocator;

		void Init( size_t blockSize, size_t blockCount, byte* pBaseAddress );

		void* Allocate( const char* filename, int line );
		FORCEINLINE void* Allocate()
		{
			return Allocate( __FILE__, __LINE__ );
		}

		void Deallocate( void* ptr );

	private:
		size_t m_blockSize;			// memory block size
		size_t m_blockCount;			// the number of reserved blocks
		size_t m_usedBlockCount;		// the number of used blocks
		size_t m_committedBytes;		// committed virtual memory bytes

		byte* m_baseAddress;		// the base address of memory blocks

		LFHBlock* m_blockBudgets;	// block data
		LFHBlock* m_freeBlockHead;
	};


	/* class LFHBucket
	*/
	LFHBucket::LFHBucket() :
		m_blockSize( 0 ),
		m_blockCount( 0 ),
		m_usedBlockCount( 0 ),
		m_committedBytes( 0 ),
		m_baseAddress( nullptr ),
		m_blockBudgets( nullptr ),
		m_freeBlockHead( nullptr )
	{

	}

	LFHBucket::~LFHBucket()
	{
		::free( m_blockBudgets );
	}

	void LFHBucket::Init( size_t blockSize, size_t blockCount, byte* pBaseAddress )
	{
		// Prevent double initialization
		if ( m_baseAddress )
		{
			return;
		}

		if ( blockSize <= 0 || blockCount <= 0 )
		{
			// throw invalid argument exception
			return;
		}

		if ( pBaseAddress == nullptr )
		{
			//throw invalid argument exception
			return;
		}

		// Force 8 byte alignment
		m_blockSize = AlignSize( blockSize, static_cast<size_t>( 8 ) );
		m_blockCount = blockCount;

		m_baseAddress = pBaseAddress;

		// Commit one page in the virtual address space
		size_t pageSize = isystem->GetPageSize();
		m_committedBytes = max( pageSize, AlignSize( m_blockSize, pageSize ) );
		m_committedBytes = min( m_committedBytes, ReservedBytes() );
		isystem->CommitVirtualMemory( m_baseAddress, m_committedBytes );

		// Initialize blocks 
		// and build the sequential free list from the blocks
		m_blockBudgets = static_cast< LFHBlock* >( ::malloc( sizeof( LFHBlock ) * m_blockCount ) );

		byte* p = m_baseAddress;
		LFHBlock* pCurrent = nullptr;
		for ( size_t i = 0; i < m_blockCount; ++i )
		{
			LFHBlock_MakeNull( m_blockBudgets[i] );
			pCurrent = &m_blockBudgets[i];
			pCurrent->Next = ( i + 1 < m_blockCount ) ? &m_blockBudgets[i + 1] : nullptr;
			pCurrent->Address = p;
#ifdef DEBUG_ALLOCATION
			pCurrent->Index = i;
			pCurrent->Prev = ( i > 0 ) ? &m_blockBudgets[i - 1] : nullptr;
#endif
			p += m_blockSize;
		}

		m_freeBlockHead = &m_blockBudgets[0];
	}

	void* LFHBucket::Allocate( const char* filename, int line )
	{
		if ( m_freeBlockHead )
		{
			LFHBlock* current = m_freeBlockHead;
			Assert( current->Next );

			// Commit a region of pages as the last committed page size in the virtual address space
			if ( current->Address + m_blockSize >= m_baseAddress + m_committedBytes )
			{
				size_t growBytes = AlignSize( m_committedBytes, static_cast<size_t>( isystem->GetPageSize() ) );
				size_t reservedBytes = ReservedBytes();
				if ( m_committedBytes + growBytes > reservedBytes )
				{
					growBytes = reservedBytes - m_committedBytes;
				}

				if ( growBytes == 0 )
				{
					// Is this OK?
					VerifyNoEntry();
					return nullptr;
				}

				if ( !isystem->CommitVirtualMemory( current->Address, growBytes ) )
				{
					throw std::bad_alloc();
				}
				m_committedBytes += growBytes;
			}

			++m_usedBlockCount;

#ifdef DEBUG_ALLOCATION
			current->FileName = filename;
			current->Line = line;
			current->Used = true;
#else
			Unused( filename, line );
#endif

			// Move the current cursor to next and return free address
			m_freeBlockHead = current->Next;
			current->Next = nullptr;	// unlink

			return current->Address;
		}

		return nullptr;
	}

	void LFHBucket::Deallocate( void* ptr )
	{
		byte* p = reinterpret_cast<byte*>( ptr );

		/*
		int blockIndex = 0;

		// Find the block which has the specified address
		for ( int i = 0; i < m_blockCount; ++i )
		{
		if ( m_pBlockBudgets[i].pAddress == ptr )
		{
		blockIndex = i;
		break;
		}
		}
		int diff = p - m_baseAddress;
		assert( blockIndex == ( diffAddr / m_blockSize ) );
		*/

		// Another implementation is...
		// Find the block by using address range (constant time search)
		int diff = p - m_baseAddress;
		int blockIndex = diff / m_blockSize;
		
#if 1
		// Make the block the head of the free list
		LFHBlock* current = &m_blockBudgets[blockIndex];
		current->Next = m_freeBlockHead;
#ifdef DEBUG_ALLOCATION
		current->FileName = nullptr;
		current->Line = -1;
		current->Used = false;
#endif
		m_freeBlockHead = current;

#else
		// FIXME: bugbugbug

		// Insert into the nearest free block in virtual memory space
		LFHBlock* pCur = &m_blockBudgets[blockIndex];	
		LFHBlock* pPrev = nullptr;
		LFHBlock* pNext = m_freeBlockHead;
		while ( pNext )
		{
			if ( p > pNext->Address )
			{
				pPrev = pNext;
				assert( pNext != pNext->Next && "unexpected circular links" );
				pNext = pNext->Next;
			}
			else
			{
				break;
			}
		}

		assert( pCur->Next != pNext && "unexpected circular links" );
		pCur->Next = pNext;

		if ( pPrev )
		{
			assert( pPrev->Next != pCur && "unexpected circular links" );
			pPrev->Next = pCur;
		}
		else
		{
			assert( m_freeBlockHead != pCur && "unexpected circular links" );
			m_freeBlockHead = pCur;
		}
#endif

		--m_usedBlockCount;
	}

	void LFHBucket::PrintMemoryUsage() const
	{
#ifdef DEBUG_ALLOCATION
		printf( "<%d> %d / %d\n", BlockSize(), UsedBlockCount(), BlockCount() );

		if ( UsedBlockCount() > 0 )
		{
			for ( size_t i = 0; i < m_blockCount; ++i )
			{
				if ( m_blockBudgets[i].Used )
				{
					printf( "\t%x, %s (%d)%\n", m_blockBudgets[i].Address, m_blockBudgets[i].FileName, m_blockBudgets[i].Line );
				}
			}
		}
#endif			
	}


#undef new

	
	/* class LFHAllocator
	*/
	class LFHBucket;

	class LFHAllocator final : public IAllocable
	{
		NONCOPYABLE_CLASS( LFHAllocator );

	public:
		enum { BucketCount = 128 };
		enum { MaxBlockSizeInBytes = 0x4000 };	// 16k limit

		enum HeapCompatibilities
		{
			ThreadSafe = 0x1
		};

		void* operator new( size_t size );
		void operator delete( void* ptr );

	public:
		explicit LFHAllocator( const char* name );
		virtual ~LFHAllocator();

		// IAllocable functions
		virtual void* Allocate( int size );
		virtual void* Allocate( int size, const char* filename, int line );
		// void* Reallocate(...)
		virtual void Deallocate( void* ptr );

		// Verify the specified pointer address is in this allocator.
		virtual bool IsValidHeap( void* ptr );

		virtual void InstallAllocFailHandler( FpAllocFailHandler fp );

		virtual void PrintMemoryUsage() const;

		FORCEINLINE size_t TotalMemoryInBytes() const
		{
			return m_bytesPerBucket * BucketCount; 
		}
		
		FORCEINLINE const char* GetName() const 
		{
			return m_name; 
		}

		// bytesPerBucket would be aligned by system's allocation granularity.
		void Init( size_t bytesPerBucket );
		
	private:
		// Get the best-fit sized bucket
		LFHBucket* GetBestFitBucket( size_t size );

		// Find the bucket by the specified address using memory range
		LFHBucket* FindBucketByAddress( void* ptr );

	private:
		size_t m_bytesPerBucket;
		byte* m_baseAddress;
		byte* m_endAddress;
		char* m_name;
		LFHBucket m_buckets[BucketCount];
		FpAllocFailHandler m_allocFailHandler;		
		SpinLock m_lock;

		// static handlers
	public:
		static bool InitAllocator( int capacityInBytes, bitflags heapCompatibilities );
		static void ShutdownAllocator();

		static FORCEINLINE LFHAllocator* GetGlobalAllocator()
		{
			// The first allocator is always the global allocator.
			return s_LFHAllocators[0];
		}

		static FORCEINLINE LFHAllocator* GetThreadLocalAllocator()
		{
			static thread_local LFHAllocator* TlsAllocator = nullptr;			
			if ( TlsAllocator == nullptr )
			{
				char buff[32] = { 0 };
				sprintf_s( buff, 32, "%u", Thread::CurrentThreadID() );
				TlsAllocator = new LFHAllocator( buff );
				TlsAllocator->Init( Max( 8 * 1024 * 1024 / LFHAllocator::BucketCount, static_cast< int >( MaxBlockSizeInBytes ) ) );
				
				// Push it into the container.
				s_LFHAllocators.Add( TlsAllocator );
			}
			return TlsAllocator;
		}

		static FORCEINLINE LFHAllocator* FindAllocatorByAddress( void* ptr )
		{
			for ( size_t i = 0; i < s_LFHAllocators.Size(); ++i )
			{
				LFHAllocator* allocator = s_LFHAllocators[i];
				if ( allocator && allocator->IsValidHeap( ptr ) )
				{
					return allocator;
				}
			}
			
			return nullptr;
		}

	private:
		static ArrayMT<LFHAllocator*> s_LFHAllocators;
	};
	
	void* LFHAllocator::operator new( size_t size )
	{
		return ::malloc( size );
	}

	void LFHAllocator::operator delete( void* ptr )
	{
		::free( ptr );
	}

	LFHAllocator::LFHAllocator( const char* name ) :
		m_bytesPerBucket( 0 ),
		m_allocFailHandler( nullptr ),
		m_baseAddress( nullptr ),
		m_endAddress( nullptr )
	{
		//Unused( name );
		//size_t alignedSize = AlignSize( ::strlen( name ) + 1, static_cast<size_t>( 8 ) );
		m_name = ::_strdup( name );
	}

	LFHAllocator::~LFHAllocator()
	{
		//PrintMemoryUsage();

		isystem->ReleaseVirtualMemory( m_baseAddress );
		m_baseAddress = nullptr;
		m_endAddress = nullptr;

		::free( m_name );
		m_name = nullptr;
	}

	void* LFHAllocator::Allocate( int size )
	{
		for ( ;; )
		{
			void* p = nullptr;

			// Use CRT malloc if size is larger than maximum block size
			if ( size > MaxBlockSizeInBytes )
			{
				p = ::malloc( size );
			}
			else if ( LFHBucket* pBucket = GetBestFitBucket( size ) )
			{
				{
					LockGuard<SpinLock> guard( m_lock );
					p = pBucket->Allocate();
				}

				if ( p == nullptr )
				{
					p = ::malloc( size );
				}
			}
			else
			{
				VerifyNoEntry();
				return ::malloc( size );
			}

			if ( p )
			{
				return p;
			}

			// call new handler
			if ( m_allocFailHandler == nullptr ||
				!m_allocFailHandler() )
			{
				throw std::bad_alloc();
			}
		}
	}

	void* LFHAllocator::Allocate( int size, const char* filename, int line )
	{
		for ( ;; )
		{
			void* p = nullptr;

			// Use CRT malloc if size is larger than maximum block size
			if ( size > MaxBlockSizeInBytes )
			{
				p = ::malloc( size );
			}
			else if ( LFHBucket* pBucket = GetBestFitBucket( size ) )
			{
				{
					LockGuard<SpinLock> guard( m_lock );
					p = pBucket->Allocate( filename, line );
				}

				if ( p == nullptr )
				{
					p = ::malloc( size );
				}
			}
			else
			{
				VerifyNoEntry();
				return ::malloc( size );
			}

			if ( p )
			{
				return p;
			}

			// call new handler
			if ( m_allocFailHandler == nullptr ||
				!m_allocFailHandler() )
			{
				throw std::bad_alloc();
			}
		}
	}

	void LFHAllocator::Deallocate( void* ptr )
	{
		if ( ptr == nullptr )
		{
			return;
		}

		// Find a bucket using range of address
		// and then deallocate memory
		if ( LFHBucket* pBucket = FindBucketByAddress( ptr ) )
		{
			// If we does not use thread affinity, Allocate() and Deallocate() would be called on different threads.
			// So we need a simple and fast lock such as SpinLock.
			// TODO: Could we move to lock-free?
			LockGuard<SpinLock> guard( m_lock );
			pBucket->Deallocate( ptr );
		}
		else
		{
			::free( ptr );
		}
	}

	bool LFHAllocator::IsValidHeap( void* ptr )
	{
		byte* p = static_cast< byte* >( ptr );
		return ( ( p >= m_baseAddress ) && ( p < m_endAddress ) );
	}

	void LFHAllocator::InstallAllocFailHandler( FpAllocFailHandler fp )
	{
		m_allocFailHandler = fp;
	}

	void LFHAllocator::PrintMemoryUsage() const
	{
		for ( int i = 0; i < BucketCount; ++i )
		{
			const LFHBucket& bucket = m_buckets[i];
			bucket.PrintMemoryUsage();
		}
	}

	void LFHAllocator::Init( size_t bytesPerBucket )
	{
		if ( bytesPerBucket < MaxBlockSizeInBytes )
		{
			//throw invalid argument
			return;
		}

		// Force system page sized byte alignment
		m_bytesPerBucket = AlignSize( bytesPerBucket, isystem->GetPageSize() );

		// Reserve virtual memory
		m_baseAddress = static_cast< byte* >( isystem->ReserveVirtualMemory( m_bytesPerBucket * BucketCount ) );

		const int Granularities[] = { 8, 16, 32, 64, 128, 256, 512 };
		const int Capacities[] = { 256, 512, 1024, 2048, 4096, 8192, 16384 };

		int bucketIdx = 0;
		int blockSize = 0;
		for ( int i = 0; i < 7; ++i )
		{
			while ( blockSize < Capacities[i] )
			{
				blockSize += Granularities[i];

				//Assert( m_bytesPerBucket % blockSize == 0 );

				m_buckets[bucketIdx].Init(
					blockSize,						// block size
					m_bytesPerBucket / blockSize,	// block count (internal fragmentation occurs because m_bytesPerBucket % blockSize != 0)
					m_baseAddress + m_bytesPerBucket * bucketIdx );	// base address of the bucket
				++bucketIdx;
			}
		}

		m_endAddress = m_baseAddress + TotalMemoryInBytes();
	}
	
	LFHBucket* LFHAllocator::GetBestFitBucket( size_t size )
	{
		// Fast search using predefined ranges
		const size_t BucketIndicies[] = { 31, 47, 65, 79, 95, 111, 127 };
		const size_t Capacities[] = { 256, 512, 1024, 2048, 4096, 8192, 16384 };

		for ( int i = 0; i < 7; ++i )
		{
			if ( size <= Capacities[i] )
			{
				for ( size_t bucket = ( i == 0 ) ? 0 : BucketIndicies[i - 1]; bucket < BucketIndicies[i]; ++bucket )
				{
					if ( size <= m_buckets[bucket].BlockSize() )
					{
						return &m_buckets[bucket];
					}
				}
			}
		}

		return nullptr;
	}

	LFHBucket* LFHAllocator::FindBucketByAddress( void* ptr )
	{
		size_t diff = static_cast< byte* >( ptr ) - m_baseAddress;
		size_t index = diff / m_bytesPerBucket;
		if ( index >= 0 && index < BucketCount )
		{
			return &m_buckets[index];
		}
		else
		{
			return nullptr;
		}
	}

	


	/* Global functions
	*/
	ArrayMT<LFHAllocator*> LFHAllocator::s_LFHAllocators;

	IAllocable* GetGlobalLFHAllocable()
	{
		return LFHAllocator::GetGlobalAllocator();
	}

	IAllocable* GetThreadLocalLFHAllocable()
	{
		return LFHAllocator::GetThreadLocalAllocator();
	}

	bool LFHAllocator::InitAllocator( int capacityInBytes, bitflags heapCompatibilities )
	{
		if ( s_LFHAllocators.Size() > 0 )
		{
			return false;
		}

		LFHAllocator* allocator = new LFHAllocator( "global" );
		allocator->Init( Max( capacityInBytes / LFHAllocator::BucketCount, static_cast<int>( MaxBlockSizeInBytes ) ) );
		s_LFHAllocators.Add( allocator );

		InstallAllocableGetter( &GetGlobalLFHAllocable );

		if ( heapCompatibilities & LFHAllocator::ThreadSafe )
		{
			InstallThreadLocalAllocatorGetter( &GetThreadLocalLFHAllocable );
		};

		return true;
	}

	void LFHAllocator::ShutdownAllocator()
	{
		for ( size_t i = 0; i < s_LFHAllocators.Size(); ++i )
		{
			delete s_LFHAllocators[i];
		}
		s_LFHAllocators.Purge();
	}


	/* global functions
	*/	
	void InitAllocator()
	{
		LFHAllocator::InitAllocator( 32 * 1024 * 1024, LFHAllocator::ThreadSafe );
	}
	
	void ShutdownAllocator()
	{
		LFHAllocator::ShutdownAllocator();
	}

	// Find an allocator that include the specified address.
	IAllocable* FindAllocatorByAddress( void* ptr )
	{
		return LFHAllocator::FindAllocatorByAddress( ptr );
	}
}



#ifdef MEMORY_OVERRIDE

/* new-delete operator overloading
*/
void* operator new( size_t size, kih::IAllocable* allocable )
{
	if ( allocable )
	{
		return allocable->Allocate( size );
	}

	return ::malloc( size );
}

void* operator new[]( size_t size, kih::IAllocable* allocable )
{
	if ( allocable )
	{
		return allocable->Allocate( size );
	}

	return ::malloc( size );
}

void* operator new( size_t size, kih::IAllocable* allocable, const char* filename, int line )
{
	if ( allocable )
	{
		return allocable->Allocate( size, filename, line );
	}

	return ::malloc( size );
}

void* operator new[]( size_t size, kih::IAllocable* allocable, const char* filename, int line )
{
	if ( allocable )
	{
		return allocable->Allocate( size, filename, line );
	}

	return ::malloc( size );
}

void operator delete( void* ptr )
{
	kih::IAllocable* allocable = kih::FindAllocatorByAddress( ptr );
	if ( allocable )
	{
		Assert( allocable->IsValidHeap( ptr ) );
		allocable->Deallocate( ptr );
		return;
	}

	::free( ptr );
}

void operator delete[]( void* ptr )
{
	kih::IAllocable* allocable = kih::FindAllocatorByAddress( ptr );
	if ( allocable )
	{
		Assert( allocable->IsValidHeap( ptr ) );
		allocable->Deallocate( ptr );
		return;
	}

	::free( ptr );
}

void operator delete( void* ptr, kih::IAllocable* allocable )
{
	if ( allocable )
	{
		Assert( allocable->IsValidHeap( ptr ) );
		allocable->Deallocate( ptr );
		return;
	}

	::free( ptr );
}

void operator delete[]( void* ptr, kih::IAllocable* allocable )
{
	if ( allocable )
	{
		Assert( allocable->IsValidHeap( ptr ) );
		allocable->Deallocate( ptr );
		return;
	}

	::free( ptr );
}

void operator delete( void* ptr, kih::IAllocable* allocable, const char* filename, int line )
{
	if ( allocable )
	{
		Assert( allocable->IsValidHeap( ptr ) );
		allocable->Deallocate( ptr );
		return;
	}

	::free( ptr );
	
	kih::Unused( filename, line );
}

void operator delete[]( void* ptr, kih::IAllocable* allocable, const char* filename, int line )
{
	if ( allocable )
	{
		Assert( allocable->IsValidHeap( ptr ) );
		allocable->Deallocate( ptr );
		return;
	}

	::free( ptr );

	kih::Unused( filename, line );
}

#else
// nothing 
#endif	// MEMORY_OVERRIDE



DEFINE_COMMAND( memory )
{
	if ( kih::IAllocable* allocable = kih::GetGlobalAllocator() )
	{
		allocable->PrintMemoryUsage();
	}
}
