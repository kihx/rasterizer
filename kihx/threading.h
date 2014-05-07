#pragma once

#include "base.h"
#include "stdsupport.h"

#include <stdexcept>
#include <queue>


#define thread_local		__declspec( thread )


namespace kih
{
	/* using ThreadFunc
	*/
	using ThreadFunc = std::function<void()>;


	// Wrapper functions for threading.
	// Do NOT directly call them. Use threading classes.
	char f_interlockedexchange8( volatile char* target, char value );
	short f_interlockedexchange16( volatile short* target, short value );
	int f_interlockedexchange32( volatile int* target, int value );
	int64 f_interlockedexchange64( volatile int64* target, int64 value );

	short f_interlockedincrement16( volatile short* target );
	int f_interlockedincrement32( volatile int* target );
	int64 f_interlockedincrement64( volatile int64* target );

	short f_interlockeddecrement16( volatile short* target );
	int f_interlockeddecrement32( volatile int* target );
	int64 f_interlockeddecrement64( volatile int64* target );

	int f_interlockedadd32( volatile int* target, int value );
	int64 f_interlockedadd64( volatile int64* target, int64 value );

	short f_interlockedcompareexchange16( volatile short* destination, short exchange, short comparand );
	int f_interlockedcompareexchange32( volatile int* destination, int exchange, int comparand );
	int64 f_interlockedcompareexchange64( volatile int64* destination, int64 exchange, int64 comparand );
	
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



	// interlock-function overrides using template specilization
	template<class T, int = sizeof( T ) * 8>
	struct interlock_func_impl
	{
	};

	#define DEFINE_INTERLOCK_FUNC( Size ) \
	template<class T>\
	struct interlock_func_impl<T, Size>\
	{\
		static FORCEINLINE T f_interlockedexchange( volatile T* target, T value )\
		{\
			return f_interlockedexchange##Size( target, value );\
		}\
		static FORCEINLINE T f_interlockedcompareexchange( volatile T* target, T value, T comparand )\
		{\
		return f_interlockedcompareexchange##Size( target, value, comparand );\
		}\
		static FORCEINLINE T f_interlockedincrement( volatile T* target )\
		{\
			return f_interlockedincrement##Size( target );\
		}\
		static FORCEINLINE T f_interlockeddecrement( volatile T* target )\
		{\
			return f_interlockeddecrement##Size( target );\
		}\
		static FORCEINLINE T f_interlockedadd( volatile T* target, T value )\
		{\
			return f_interlockedadd##Size( target, value );\
		}\
	};

	DEFINE_INTERLOCK_FUNC( 16 );	// short
	DEFINE_INTERLOCK_FUNC( 32 );	// int
	DEFINE_INTERLOCK_FUNC( 64 );	// int64


	/* class Atomic
	*/
	template<class T, bool = std::is_integral<T>::value>
	class Atomic
	{
		static_assert( std::is_integral<T>::value, "only integral type is allowed" );
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
	
	// integral specialization
	template<class T>
	class Atomic<T, true>
	{
		NONCOPYABLE_CLASS( Atomic );
		
	public:
		FORCEINLINE Atomic() :
			m_value( 0 )
		{
		}

		FORCEINLINE Atomic( T value ) :
			m_value( value )
		{
		}

		FORCEINLINE T Value() const
		{
			return m_value;
		}

		FORCEINLINE T FetchAndIncrement()
		{
			T old = Value();
			interlock_func_impl<T>::f_interlockedincrement( &m_value );
			return old;
		}

		FORCEINLINE T FetchAndDecrement()
		{
			T old = Value();
			interlock_func_impl<T>::f_interlockeddecrement( &m_value );
			return old;
		}

		FORCEINLINE operator T() const
		{
			return Value();
		}

		FORCEINLINE void Exchange( T value ) volatile
		{
			interlock_func_impl<T>::f_interlockedexchange( &m_value, value );
		}

		FORCEINLINE T CompareExchange( T value, T comparand ) volatile
		{
			return interlock_func_impl<T>::f_interlockedcompareexchange( &m_value, value, comparand );
		}

		FORCEINLINE T operator++( )
		{
			return interlock_func_impl<T>::f_interlockedincrement( &m_value );
		}

		FORCEINLINE T operator++( ) volatile
		{
			return interlock_func_impl<T>::f_interlockedincrement( &m_value );
		}

		FORCEINLINE T operator--( )
		{
			return interlock_func_impl<T>::f_interlockeddecrement( &m_value );
		}

		FORCEINLINE T operator--( ) volatile
		{
			return interlock_func_impl<T>::f_interlockeddecrement( &m_value );
		}

		FORCEINLINE T operator=( T value ) volatile
		{
			Exchange( value );
			return m_value;
		}

		FORCEINLINE T operator+=( T arg )
		{
			interlock_func_impl<T>::f_interlockedadd( &m_value, arg );
			return m_value;
		}

		FORCEINLINE T operator+=( T arg ) volatile
		{
			interlock_func_impl<T>::f_interlockedadd( &m_value, arg );
			return m_value;
		}

		FORCEINLINE T operator-=( T arg )
		{
			interlock_func_impl<T>::f_interlockedadd( &m_value, -arg );
			return m_value;
		}

		FORCEINLINE T operator-=( T arg ) volatile
		{
			interlock_func_impl<T>::f_interlockedadd( &m_value, -arg );
			return m_value;
		}

	private:
		volatile T m_value;
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

	
	/* class SpinLock
	*/
	class SpinLock final
	{
	public:
		SpinLock() = default;
		~SpinLock() = default;

		void Lock();
		void Unlock();

	private:
		Atomic<int> m_atom;
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

		template<class Func, class... Args>
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

		static bool IsInMainThread();

		static unsigned int CurrentThreadID();
		
		static int HardwareConcurrency();

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


	/* class ThreadPool
	*/
	class ThreadPool final : public Singleton<ThreadPool>
	{
		NONCOPYABLE_CLASS( ThreadPool );

		friend class Singleton<ThreadPool>;
		friend class TaskThread;

	private:
		ThreadPool();
	public:
		~ThreadPool();

		FORCEINLINE size_t QueuedWorkCount() const
		{
			return m_taskQueue.size();
		}

		FORCEINLINE size_t MaxConcurrency() const
		{
			return m_threadPool.size();
		}

		void Queue( ThreadFunc funcWork );

		void WaitForAllTasks();

		ThreadFunc FinishTaskAndGetNext( TaskThread* thread );

	private:
		std::queue<ThreadFunc> m_taskQueue;
		std::queue<TaskThread*> m_threadQueue;	// available threads to work
		StlVector<std::shared_ptr<TaskThread>> m_threadPool;
		Mutex m_mutex;
	};

	
	/* Concurrency iteration
	*/
	FORCEINLINE void ForEachTaskInParallel( size_t begin, size_t end, ThreadFunc task )
	{
		ThreadPool* threadPool = ThreadPool::GetInstance();
		for ( size_t i = begin; i < end; ++i )
		{
			threadPool->Queue( task );
		}
		threadPool->WaitForAllTasks();
	}
};

using kih::Atomic;
using kih::Event;
using kih::Mutex;
using kih::Thread;
using kih::ThreadPool;
