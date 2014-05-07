#pragma once

#define MEMORY_OVERRIDE
#define DEBUG_ALLOCATION


namespace kih
{
	typedef bool( *FpAllocFailHandler )( );

	/* class IAllocable
	*/
	class IAllocable
	{
	public:
		virtual void* Allocate( int size ) = 0;
		virtual void* Allocate( int size, const char* filename, int line ) = 0;

		// void* Reallocate(...)

		virtual void Deallocate( void* ptr ) = 0;

		// Verify the specified pointer address is in this allocable.
		virtual bool IsValidHeap( void* ptr ) = 0;

		virtual void InstallAllocFailHandler( FpAllocFailHandler fp ) = 0;

		virtual void PrintMemoryUsage() const = 0;
	};

	// Get the global allocator in the main thread or a thread-local allocator in a secondary thread.
	IAllocable* GetCurrentAllocator();

	void InitAllocator();
	void ShutdownAllocator();
};



#ifdef MEMORY_OVERRIDE

namespace kih
{
	// Allocator for STL containers.	
	template<class T>
	class StlAllocator
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

	public:
		// convert T to U
		template<class U> 
		struct rebind 
		{
			typedef StlAllocator<U> other;
		};

	public:
		// address
		FORCEINLINE pointer address( reference r ) 
		{
			return &r; 
		}

		FORCEINLINE const_pointer address( const_reference r ) 
		{
			return &r; 
		}

		// allocation
		FORCEINLINE pointer allocate( size_type count )
		{
#ifdef DEBUG_ALLOCATION
			return reinterpret_cast< pointer >( GetCurrentAllocator()->Allocate( count * sizeof( T ), __FILE__, __LINE__ ) );
#else
			return reinterpret_cast< pointer >( GetCurrentAllocator()->Allocate( count * sizeof( T ) ) );
#endif
		}

		FORCEINLINE pointer allocate( size_type count, const void* )
		{
			Unused( other );
			return allocate( count );
		}

		FORCEINLINE void deallocate( pointer ptr, size_type )
		{
			::operator delete( ptr );
		}

		FORCEINLINE void construct( pointer ptr, const value_type& t )
		{
			::new( ptr ) T( t );
		}

		template<class U, class... Args>
		FORCEINLINE void construct( pointer ptr, Args&&... args )
		{
			::new( ptr ) U( std::forward<Args>( args )... );
		}

		FORCEINLINE void destroy( pointer ptr )
		{
			Unused( ptr );
			ptr->~T();
		}

		template<class U>
		FORCEINLINE void destroy( U* ptr )
		{
			Unused( ptr );
			ptr->~U();
		}

		FORCEINLINE size_type max_size() const
		{
			return static_cast<size_t>( -1 );
		}
	};
}


/* new-delete operator overloading
*/
// general allocation
void* operator new( size_t size, kih::IAllocable* allocable );
void* operator new[]( size_t size, kih::IAllocable* allocable );

// support allocation debugging
void* operator new( size_t size, kih::IAllocable* allocable, const char* filename, int line );
void* operator new[]( size_t size, kih::IAllocable* allocable, const char* filename, int line );

// default delete
void operator delete( void* ptr );
void operator delete[]( void* ptr );

// placement delete pairs
void operator delete( void* ptr, kih::IAllocable* allocable );
void operator delete[]( void* ptr, kih::IAllocable* allocable );

void operator delete( void* ptr, kih::IAllocable* allocable, const char* filename, int line );
void operator delete[]( void* ptr, kih::IAllocable* allocable, const char* filename, int line );


#undef new

#ifdef DEBUG_ALLOCATION
#define NEW_LFH_DEBUG new( kih::GetCurrentAllocator(), __FILE__, __LINE__ )
#define new NEW_LFH_DEBUG
#else
#define NEW_LFH_STD new( kih::GetCurrentAllocator() )
#define new NEW_LFH_STD
#endif	// DEBUG_ALLOCATION

#else

#endif	// MEMORY_OVERRIDE

