#pragma once

//#define MEMORY_OVERRIDE

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

		virtual void InstallAllocFailHandler( FpAllocFailHandler fp ) = 0;
	};

	typedef IAllocable* ( *FpAllocatorGetter )( );
	void InstallGlobalAllocatorGetter( FpAllocatorGetter fp );
	IAllocable* GetGlobalAllocator();
};


#ifdef MEMORY_OVERRIDE

/* new-delete operator overloading
*/
// general allocation
void* operator new( size_t size, kih::IAllocable* pAllocable );
void* operator new[]( size_t size, kih::IAllocable* pAllocable );

// support allocation debugging
void* operator new( size_t size, kih::IAllocable* pAllocable, const char* filename, int line );
void* operator new[]( size_t size, kih::IAllocable* pAllocable, const char* filename, int line );

// default delete
void operator delete( void* ptr );
void operator delete[]( void* ptr );

// placement delete pairs
void operator delete( void* ptr, kih::IAllocable* pAllocable );
void operator delete[]( void* ptr, kih::IAllocable* pAllocable );

void operator delete( void* ptr, kih::IAllocable* pAllocable, const char* filename, int line );
void operator delete[]( void* ptr, kih::IAllocable* pAllocable, const char* filename, int line );


#undef new

#ifdef DEBUG_ALLOCATION
#define NEW_DEBUG_ALLOCATION new( kih::GetGlobalAllocator(), __FILE__, __LINE__ )
#define new NEW_DEBUG_ALLOCATION
#else
#define NEW_LFH_STD new( kih::GetGlobalAllocator() )
#define new NEW_LFH_STD
#endif

#else
// nothing 
#endif	// MEMORY_OVERRIDE
