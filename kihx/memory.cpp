#include "stdafx.h"
#include "concommand.h"
#include "mathlib.h"
#include "memory.h"
#include "system.h"


namespace kih
{
	/* Global allocator accessors
	*/
	typedef IAllocable* ( *FpAllocatorGetter )( );

	static FpAllocatorGetter s_FpAllocableGetter;

	void InstallGlobalAllocatorGetter( FpAllocatorGetter fp )
	{
		s_FpAllocableGetter = fp;
	}

	IAllocable* GetGlobalAllocator()
	{
		return s_FpAllocableGetter ? s_FpAllocableGetter() : nullptr;
	}


	/* Low Fragmentation Heap (LFH)
	*/
	struct LFHBlock
	{
		LFHBlock* Next;		// next block pointer
		void* Address;		// heap address
#ifdef DEBUG_ALLOCATION
		const char* FileName;
		int Line;
#endif

		void Reset()
		{
#ifdef DEBUG_ALLOCATION
			FileName = "";
			Line = -1;
#endif	
			Address = nullptr;
			Next = nullptr;
		}
	};

	class LFHBucket
	{
	public:
		LFHBucket();
		~LFHBucket();

		int BlockSize() const
		{
			return m_blockSize;
		}

		int BlockCount() const
		{
			return m_blockCount;
		}

		int UsedBlockCount() const
		{
			return m_usedBlockCount;
		}

		// Returns reserved heap size of this bucket in bytes
		int ReservedBytes() const
		{
			return m_blockSize * m_blockCount;
		}

		// Returns committed heap size of this bucket in bytes
		int CommittedBytes() const
		{
			return m_committedBytes;
		}

		int AllocatedBytes() const
		{
			return BlockCount() * UsedBlockCount();
		}

		bool IsFull() const
		{
			return m_freeBlockHead == nullptr;
		}

		void PrintMemoryUsage() const;

		// only call by LFHAllocator
	private:
		friend class LFHAllocator;

		void Init( int blockSize, int blockCount, byte* pBaseAddress );

		void* Allocate( const char* filename, int line );
		FORCEINLINE void* Allocate()
		{
			return Allocate( __FILE__, __LINE__ );
		}

		void Deallocate( void* ptr );

	private:
		int m_blockSize;			// memory block size
		int m_blockCount;			// the number of reserved blocks
		int m_usedBlockCount;		// the number of used blocks
		int m_committedBytes;		// committed virtual memory bytes

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

	void LFHBucket::Init( int blockSize, int blockCount, byte* pBaseAddress )
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
		m_blockSize = AlignSize( blockSize, 8 );
		m_blockCount = blockCount;

		m_baseAddress = pBaseAddress;

		// Commit one page in the virtual address space
		int pageSize = isystem->GetPageSize();
		m_committedBytes = max( pageSize, AlignSize( m_blockSize, pageSize ) );
		m_committedBytes = min( m_committedBytes, ReservedBytes() );
		isystem->CommitVirtualMemory( m_baseAddress, m_committedBytes );

		// Initialize blocks 
		// and build the sequential free list from the blocks
		m_blockBudgets = static_cast< LFHBlock* >( ::malloc( sizeof( LFHBlock ) * m_blockCount ) );

		byte* p = m_baseAddress;
		m_freeBlockHead = &m_blockBudgets[0];
		LFHBlock* pCurrent = m_freeBlockHead;
		for ( int i = 0; i < m_blockCount; ++i )
		{
			pCurrent = &m_blockBudgets[i];
			pCurrent->Reset();

			pCurrent->Next = ( i + 1 < m_blockCount ) ? &m_blockBudgets[i + 1] : nullptr;
			pCurrent->Address = p;
			p += m_blockSize;
		}
	}

	void* LFHBucket::Allocate( const char* filename, int line )
	{
		if ( m_freeBlockHead )
		{
			LFHBlock* pCurrent = m_freeBlockHead;

			// Commit a region of pages as the last committed page size in the virtual address space
			if ( pCurrent->Address >= m_baseAddress + m_committedBytes )
			{
				int growBytes = AlignSize( m_committedBytes, isystem->GetPageSize() );
				int reservedBytes = ReservedBytes();
				if ( m_committedBytes + growBytes > reservedBytes )
				{
					growBytes = reservedBytes - m_committedBytes;
				}

				if ( !isystem->CommitVirtualMemory( pCurrent->Address, growBytes ) )
				{
					throw std::bad_alloc();
				}
				m_committedBytes += growBytes;
			}

			++m_usedBlockCount;

#ifdef DEBUG_ALLOCATION
			m_freeBlockHead->FileName = filename;
			m_freeBlockHead->Line = line;
#endif

			// Move the current cursor to next and return free address
			m_freeBlockHead = pCurrent->Next;
			pCurrent->Next = nullptr;	// invalidate
			return pCurrent->Address;
		}

		return nullptr;
	}

	void LFHBucket::Deallocate( void* ptr )
	{
		byte* p = ( byte* ) ptr;

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
		int diff = p - m_pBaseAddress;
		assert( blockIndex == ( diffAddr / m_blockSize ) );
		*/

		// Another implementation is...
		// Find the block by using address range (constant time search)
		int diff = p - m_baseAddress;
		int blockIndex = diff / m_blockSize;

#if 1
		// Make the block the head of the free list
		LFHBlock* pHead = m_freeBlockHead;
		m_freeBlockHead = &m_blockBudgets[blockIndex];
		m_freeBlockHead->Next = pHead;

#ifdef DEBUG_ALLOCATION
		m_freeBlockHead->FileName = nullptr;
		m_freeBlockHead->Line = -1;
#endif

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
		printf( "<%d> %d / %d\n", BlockSize(), UsedBlockCount(), BlockCount() );

		if ( UsedBlockCount() > 0 )
		{
			for ( int i = 0; i < m_blockCount; ++i )
			{
				if ( m_blockBudgets[i].Next == nullptr )
				{
#ifdef DEBUG_ALLOCATION
					printf( "\t%x, %s (%d)%\n", m_blockBudgets[i].Address, m_blockBudgets[i].FileName, m_blockBudgets[i].Line );
#else
					printf( "\t%x\n", m_blockBudgets[i].Address );
#endif			
				}
			}
		}
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

		virtual void InstallAllocFailHandler( FpAllocFailHandler fp );

		virtual void PrintMemoryUsage() const;

		// bytesPerBucket is aligned by system's allocation granularity.
		void Init( int bytesPerBucket );

		int TotalMemoryInBytes() const { return m_bytesPerBucket * BucketCount; }

	private:
		// Get the best-fit sized bucket
		LFHBucket* GetBestFitBucket( int size );

		// Find the bucket by the specified address using memory range
		LFHBucket* FindBucketByAddress( void* ptr );

	private:
		int m_bytesPerBucket;
		byte* m_pBaseAddress;
		LFHBucket m_buckets[BucketCount];
		FpAllocFailHandler m_fpAllocFailHandler;
		//String m_name;

		// global allocator
	public:
		static bool InitGlobalAllocator( int capacityInBytes, bitflags heapCompatibilities );
		static void ShutdownGlobalAllocator();
		static LFHAllocator* GetGlobalAllocator();

	private:
		static LFHAllocator* s_pGlobalInstance;
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
		m_fpAllocFailHandler( nullptr )/*,
		m_name( name )*/
	{
		Unused( name );
		//int alignedSize = AlignSize( ::strlen( name ) + 1, 8 );
	}

	LFHAllocator::~LFHAllocator()
	{
		isystem->ReleaseVirtualMemory( m_pBaseAddress );
		m_pBaseAddress = nullptr;
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
				p = pBucket->Allocate();
			}
			else
			{
				return ::malloc( size );
			}

			if ( p )
			{
				return p;
			}

			// call new handler
			if ( m_fpAllocFailHandler )
			{
				if ( !m_fpAllocFailHandler() )
				{
					throw std::bad_alloc();
				}
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
				p = pBucket->Allocate( filename, line );
			}
			else
			{
				return ::malloc( size );
			}

			if ( p )
			{
				return p;
			}

			// call new handler
			if ( m_fpAllocFailHandler )
			{
				if ( !m_fpAllocFailHandler() )
				{
					throw std::bad_alloc();
				}
			}
		}
	}

	void LFHAllocator::Deallocate( void* ptr )
	{
		if ( ptr == nullptr )
		{
			return;
		}

		// Find a bucket using address range
		// and then deallocate memory
		if ( LFHBucket* pBucket = FindBucketByAddress( ptr ) )
		{
			pBucket->Deallocate( ptr );
		}
		else
		{
			::free( ptr );
		}
	}

	void LFHAllocator::InstallAllocFailHandler( FpAllocFailHandler fp )
	{
		m_fpAllocFailHandler = fp;
	}

	void LFHAllocator::Init( int bytesPerBucket )
	{
		if ( bytesPerBucket < MaxBlockSizeInBytes )
		{
			//throw invalid argument
			return;
		}

		// Force system page sized byte alignment
		m_bytesPerBucket = AlignSize( bytesPerBucket, isystem->GetPageSize() );

		// Reserve virtual memory
		m_pBaseAddress = static_cast< byte*>( isystem->ReserveVirtualMemory( m_bytesPerBucket * BucketCount ) );

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
					m_pBaseAddress + m_bytesPerBucket * bucketIdx );	// base address of the bucket
				++bucketIdx;
			}
		}
	}

	void LFHAllocator::PrintMemoryUsage() const
	{
		for ( int i = 0; i < BucketCount; ++i )
		{
			const LFHBucket& bucket = m_buckets[i];
			bucket.PrintMemoryUsage();
		}
	}

	LFHBucket* LFHAllocator::GetBestFitBucket( int size )
	{
		// Fast search using predefined ranges
		const int BucketIndicies[] = { 31, 47, 65, 79, 95, 111, 127 };
		const int Capacities[] = { 256, 512, 1024, 2048, 4096, 8192, 16384 };

		for ( int i = 0; i < 7; ++i )
		{
			if ( size <= Capacities[i] )
			{
				for ( int bucket = ( i == 0 ) ? 0 : BucketIndicies[i - 1]; bucket < BucketIndicies[i]; ++bucket )
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
		int diff = ( byte* ) ptr - m_pBaseAddress;
		int index = diff / m_bytesPerBucket;
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
	LFHAllocator* LFHAllocator::s_pGlobalInstance = nullptr;

	IAllocable* GetLFHAllocable()
	{
		return LFHAllocator::GetGlobalAllocator();
	}

	bool LFHAllocator::InitGlobalAllocator( int capacityInBytes, bitflags heapCompatibilities )
	{
		s_pGlobalInstance = new LFHAllocator( "global" );
		s_pGlobalInstance->Init( Max( capacityInBytes / LFHAllocator::BucketCount, static_cast<int>( MaxBlockSizeInBytes ) ) );
		InstallGlobalAllocatorGetter( &GetLFHAllocable );

		Unused( heapCompatibilities );

		return true;
	}

	void LFHAllocator::ShutdownGlobalAllocator()
	{
		delete s_pGlobalInstance;
		s_pGlobalInstance = nullptr;
	}

	LFHAllocator* LFHAllocator::GetGlobalAllocator()
	{
		return s_pGlobalInstance;
	}


	/* global functions
	*/
	void InitAllocator()
	{
		LFHAllocator::InitGlobalAllocator( 32 * 1024 * 1024, 0 );
	}
	
	void ShutdownAllocator()
	{
		LFHAllocator::ShutdownGlobalAllocator();
	}
}



#ifdef MEMORY_OVERRIDE

/* new-delete operator overloading
*/
void* operator new( size_t size, kih::IAllocable* pAllocable )
{
	if ( pAllocable )
	{
		return pAllocable->Allocate( size );
	}

	return ::malloc( size );
}

void* operator new[]( size_t size, kih::IAllocable* pAllocable )
{
	if ( pAllocable )
	{
		return pAllocable->Allocate( size );
	}

	return ::malloc( size );
}

void* operator new( size_t size, kih::IAllocable* pAllocable, const char* filename, int line )
{
	if ( pAllocable )
	{
		return pAllocable->Allocate( size, filename, line );
	}

	return ::malloc( size );
}

void* operator new[]( size_t size, kih::IAllocable* pAllocable, const char* filename, int line )
{
	if ( pAllocable )
	{
		return pAllocable->Allocate( size, filename, line );
	}

	return ::malloc( size );
}

void operator delete( void* ptr )
{
	// TODO: find an appropriate allocator
	if ( kih::IAllocable* pAllocable = kih::GetGlobalAllocator() )
	{
		pAllocable->Deallocate( ptr );
		return;
	}

	::free( ptr );
}

void operator delete[]( void* ptr )
{
	// TODO: find an appropriate allocator
	if ( kih::IAllocable* pAllocable = kih::GetGlobalAllocator() )
	{
		pAllocable->Deallocate( ptr );
		return;
	}

	::free( ptr );
}

void operator delete( void* ptr, kih::IAllocable* pAllocable )
{
	if ( pAllocable )
	{
		pAllocable->Deallocate( ptr );
		return;
	}

	::free( ptr );
}

void operator delete[]( void* ptr, kih::IAllocable* pAllocable )
{
	if ( pAllocable )
	{
		pAllocable->Deallocate( ptr );
		return;
	}

	::free( ptr );
}

void operator delete( void* ptr, kih::IAllocable* pAllocable, const char* filename, int line )
{
	if ( pAllocable )
	{
		pAllocable->Deallocate( ptr );
		return;
	}

	::free( ptr );
	
	kih::Unused( filename, line );
}

void operator delete[]( void* ptr, kih::IAllocable* pAllocable, const char* filename, int line )
{
	if ( pAllocable )
	{
		pAllocable->Deallocate( ptr );
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
	if ( kih::IAllocable* pAllocable = kih::GetGlobalAllocator() )
	{
		pAllocable->PrintMemoryUsage();
	}
}