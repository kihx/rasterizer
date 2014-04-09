#pragma once

#include "base.h"

#include <stdexcept>
#include <vector>
#include <queue>

//#include <mutex>

//#define Mutex	std::mutex;
//#define LockGuard	std::lock_guard;


namespace kih
{
	// Wrapper functions for threading.
	// Do NOT directly call them. Use threading classes.
	char f_interlockedexchange8( volatile char* target, char value );
	short f_interlockedexchange16( volatile short* target, short value );
	int f_interlockedexchange32( volatile int* target, int value );
	__int64 f_interlockedexchange64( volatile __int64* target, __int64 value );

	short f_interlockedincrement16( volatile short* target );
	int f_interlockedincrement32( volatile int* target );
	__int64 f_interlockedincrement64( volatile __int64* target );

	short f_interlockeddecrement16( volatile short* target );
	int f_interlockeddecrement32( volatile int* target );
	__int64 f_interlockeddecrement64( volatile __int64* target );

	int f_interlockedadd32( volatile int* target, int value );
	__int64 f_interlockedadd64( volatile __int64* target, __int64 value );
	
	void* f_createevent( bool manualReset );
	void f_setevent( void* handle );
	void f_resetevent( void* handle );

	void* f_beginthread(
		void* security,
		unsigned int stack_size,
		unsigned int( __stdcall* start_address )( void* ),
		void* arglist,
		unsigned int initflag,
		unsigned int* thrdaddr );
	void f_endthreadex( unsigned int retval );

	void f_sleep( unsigned int ms );

	unsigned int f_waitforsingleobject( void* hHandle, unsigned int dwMilliseconds );


	/* using ThreadFunc
	*/
	using ThreadFunc = std::function<void()>;


	/* class Atomic
	*/
	template<typename T, typename std::enable_if< std::is_scalar<T>::value >::type* = nullptr>
	class Atomic
	{
		Atomic() = delete;

		//FORCEINLINE typename std::enable_if<sizeof( T ) == 2>::type
		//	Exchange( T ) volatile
		//{
		//}
	};

	template<>
	class Atomic<bool>
	{
		NONCOPYABLE_CLASS( Atomic );

	public:
		FORCEINLINE Atomic() :
			m_value( 0 )
		{
		}

		FORCEINLINE Atomic( bool value ) :
			m_value( value ? 1 : 0 )
		{
		}

		FORCEINLINE bool Value() const
		{
			return m_value != 0;
		}

		FORCEINLINE operator bool() const
		{
			return Value();
		}

		FORCEINLINE void Exchange( bool value ) volatile
		{
			f_interlockedexchange8( &m_value, value ? 1 : 0 );
		}

		FORCEINLINE bool operator=( bool value ) volatile
		{
			Exchange( value );
			return m_value != 0;
		}

	private:
		volatile char m_value;
	};	

	template<>
	class Atomic<int>
	{
		NONCOPYABLE_CLASS( Atomic );

	public:
		FORCEINLINE Atomic() :
			m_value( 0 )
		{
		}

		FORCEINLINE Atomic( int value ) :
			m_value( value )
		{
		}
		
		FORCEINLINE int Value() const
		{
			return m_value;
		}

		FORCEINLINE operator int() const
		{
			return Value();
		}

		FORCEINLINE void Exchange( int value ) volatile
		{
			f_interlockedexchange32( &m_value, value );
		}

		FORCEINLINE int operator++()
		{
			return f_interlockedincrement32( &m_value );
		}

		FORCEINLINE int operator++() volatile
		{
			return f_interlockedincrement32( &m_value );
		}

		FORCEINLINE int operator--( )
		{
			return f_interlockeddecrement32( &m_value );
		}

		FORCEINLINE int operator--( ) volatile
		{
			return f_interlockeddecrement32( &m_value );
		}

		FORCEINLINE int operator=( int value ) volatile
		{
			Exchange( value );
			return m_value;
		}

		FORCEINLINE int operator+=( int arg )
		{
			f_interlockedadd32( &m_value, arg );
			return m_value;
		}

		FORCEINLINE int operator+=( int arg ) volatile
		{
			f_interlockedadd32( &m_value, arg );
			return m_value;
		}

		FORCEINLINE int operator-=( int arg )
		{
			f_interlockedadd32( &m_value, -arg );
			return m_value;
		}

		FORCEINLINE int operator-=( int arg ) volatile
		{
			f_interlockedadd32( &m_value, -arg );
			return m_value;
		}

	private:
		volatile int m_value;
	};

	
	/* class Event
	*/
	class Event final
	{
		NONCOPYABLE_CLASS( Event );

	public:
		Event() :
			m_handle( nullptr )
		{
		}

		~Event()
		{
			if ( m_handle )
			{
				CloseHandle( m_handle );
			}
		}

		FORCEINLINE bool IsAvailable() const
		{
			return m_handle != nullptr;
		}

		void Create( bool manualReset )
		{
			if ( IsAvailable() )
			{
				std::runtime_error( "double create event error" );
			}
			m_handle = f_createevent( manualReset );
		}

		void Signal()
		{
			if ( !IsAvailable() )
			{
				std::runtime_error( "event is not available" );
			}
			f_setevent( m_handle );
		}

		void Reset()
		{
			if ( !IsAvailable() )
			{
				std::runtime_error( "event is not available" );
			}
			f_resetevent( m_handle );
		}

		bool Wait( unsigned int time = 0xFFFFFFFF )
		{
			if ( !IsAvailable() )
			{
				std::runtime_error( "event is not available" );
			}
			return f_waitforsingleobject( m_handle, time ) == WAIT_OBJECT_0;
		}

	private:
		void* m_handle;
	};


	/* class Mutex
	*/
	class Mutex final
	{
	public:
		Mutex();
		~Mutex();

		void Lock();
		void Unlock();

	private:
		void* m_mutexImpl;
	};


	/* struct ThreadInfo
	*/
	struct ThreadData
	{
		NONCOPYABLE_STRUCT( ThreadData );

		void* Handle;
		unsigned int ID;
		ThreadFunc Func;

		ThreadData() :
			Handle( nullptr ),
			ID( 0 )
		{
		}

		ThreadData( ThreadData&& other )
		{
			Handle = other.Handle;
			ID = other.ID;
			Func = other.Func;
			MakeNull( other );
		}

		FORCEINLINE ThreadData& operator=( ThreadData&& other )
		{
			Handle = other.Handle;
			ID = other.ID;
			Func = other.Func;
			MakeNull( other );
			return *this;
		}

		static FORCEINLINE void MakeNull( ThreadData& t )
		{
			t.Handle = nullptr;
			t.ID = 0;
			t.Func = nullptr;
		}
	};


	/* class Thread
	*/
	class Thread final
	{
		NONCOPYABLE_CLASS( Thread );

	public:
		Thread() :
			m_started( false )
		{
		}

		Thread( Thread&& other )
		{
			m_tdata = std::move( other.m_tdata );
			m_started.Exchange( other.m_started.Value() );
			MakeNull( other );
		}

		template<typename Func, typename... Args>
		explicit Thread( Func&& func, Args&&... args )
		{
			m_started.Exchange( false );

			auto f = std::bind( func, args... );
			m_tdata.Func = f;
			m_tdata.Handle = f_beginthread(
				nullptr,
				0,	// stack_size
				[]( void* arg ) -> unsigned int		// thread func
				{
					if ( Thread* t = reinterpret_cast< Thread* >( arg ) )
					{
						if ( t->m_tdata.Func )
						{
							t->m_started.Exchange( true );
							t->m_tdata.Func();
						}
					}
					//f_endthreadex( 0 );
					return 0;
				},
				this,	// arg
				0,
				&m_tdata.ID );

			if ( m_tdata.Handle == nullptr )
			{
				Assert( 0 && "threading error" );
				//exit( 1 );
			}

			// Wait until this thread is started.
			while ( !m_started );
		}

		~Thread()
		{
			if ( Joinable() )
			{
				std::terminate();
			}
		}

		FORCEINLINE void* Handle()
		{
			return m_tdata.Handle;
		}

		FORCEINLINE unsigned int ID()
		{
			return m_tdata.ID;
		}

		FORCEINLINE bool Joinable() const
		{
			return m_tdata.Handle != nullptr;
		}

		FORCEINLINE void Join()
		{
			//if ( m_id == Current.m_id )
			//{
			//	throw std::runtime_error( "deadlock would occur" );
			//}
			if ( !Joinable() )
			{
				throw std::runtime_error( "this thread cannot join" );
			}

			f_waitforsingleobject( m_tdata.Handle, 0xFFFFFFFF );
			MakeNull( *this );
		}

		//static Thread& Current()
		//{
		//	return *this;
		//}

		FORCEINLINE void Swap( Thread& other )
		{
			kih::Swap( m_tdata, other.m_tdata );
			
			bool tmp = other.m_started;
			m_started.Exchange( other.m_started );
			other.m_started.Exchange( tmp );
		}

		// move assignment
		FORCEINLINE Thread& operator=( Thread&& other )
		{
			m_tdata = std::move( other.m_tdata );
			m_started.Exchange( other.m_started.Value() );
			MakeNull( other );

			return *this;
		}

		// c++11 compatible
		FORCEINLINE void join()
		{
			Join();
		}

		FORCEINLINE void swap( Thread& other )
		{
			Swap( other );
		}

		static FORCEINLINE void Sleep( unsigned int ms )
		{
			f_sleep( ms );
		}

	private:
		static FORCEINLINE void MakeNull( Thread& t )
		{
			ThreadData::MakeNull( t.m_tdata );
			t.m_started.Exchange( false );
		}

	private:
		ThreadData m_tdata;
		Atomic<bool> m_started;
	};


	/* class ParallelWorker
	*/
	class ParallelWorker final
	{
		NONCOPYABLE_CLASS( ParallelWorker );

		friend class PooledThread;
		friend void ParallelWorker_OnWorkCompleted( ParallelWorker* parallel, PooledThread* thread );

	public:
		explicit ParallelWorker( size_t maxConcurrency );
		~ParallelWorker();
		
		void Queue( ThreadFunc funcWork );

		void WaitForAllTasks();

		FORCEINLINE size_t QueuedWorkCount() const
		{
			return m_taskQueue.size();
		}

		FORCEINLINE size_t MaxConcurrency() const
		{
			return m_pooledThreads.size();
		}

	private:
		std::shared_ptr<PooledThread> FindIdleThread();

	private:
		std::queue<ThreadFunc> m_taskQueue;
		std::vector<std::shared_ptr<PooledThread>> m_pooledThreads;
		Mutex m_mutex;
	};
};

using kih::Atomic;
using kih::Event;
using kih::Thread;
using kih::ParallelWorker;
