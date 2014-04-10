#include "stdafx.h"
#include "threading.h"
#include "concommand.h"

#include <windows.h>
#include <process.h>


namespace kih
{
	char f_interlockedexchange8( volatile char* target, char value )
	{
		return _InterlockedExchange8( target, value );
	}

	short f_interlockedexchange16( volatile short* target, short value )
	{
		return _InterlockedExchange16( target, value );
	}

	int f_interlockedexchange32( volatile int* target, int value )
	{
		return _InterlockedExchange( ( volatile long* ) target, value );
	}

	__int64 f_interlockedexchange64( volatile __int64* target, __int64 value )
	{
		return _InterlockedExchange64( target, value );
	}
	
	short f_interlockedincrement16( volatile short* target )
	{
		return _InterlockedIncrement16( target );
	}

	int f_interlockedincrement32( volatile int* target )
	{
		return _InterlockedIncrement( ( volatile long* ) target );
	}

	__int64 f_interlockedincrement64( volatile __int64* target )
	{
		return _InterlockedIncrement64( target );
	}

	short f_interlockeddecrement16( volatile short* target )
	{
		return _InterlockedDecrement16( target );
	}

	int f_interlockeddecrement32( volatile int* target )
	{
		return _InterlockedDecrement( ( volatile long* ) target );
	}

	__int64 f_interlockeddecrement64( volatile __int64* target )
	{
		return _InterlockedDecrement64( target );
	}

	int f_interlockedadd32( volatile int* target, int value )
	{
		return _InterlockedAdd( ( volatile long* ) target, value );
	}

	__int64 f_interlockedadd64( volatile __int64* target, __int64 value )
	{
		return _InterlockedAdd64( target, value );
	}

	void* f_createevent( bool manualReset )
	{
		return CreateEvent( NULL, manualReset ? TRUE : FALSE, FALSE, NULL );
	}

	void f_setevent( void* handle )
	{
		SetEvent( static_cast< HANDLE >( handle ) );
	}

	void f_resetevent( void* handle )
	{
		ResetEvent( static_cast< HANDLE >( handle ) );
	}

	void* f_beginthread(
		void *security,
		unsigned int stack_size,
		unsigned int( __stdcall *start_address )( void * ),
		void *arglist,
		unsigned int initflag,
		unsigned int *thrdaddr
		)
	{
		return reinterpret_cast< void* >(
			_beginthreadex( security,
			stack_size,
			start_address,
			arglist,
			initflag,
			thrdaddr
			) );
	}

	void f_endthreadex( unsigned int retval )
	{
		_endthreadex( retval );
	}

	void f_sleep( unsigned int ms )
	{
		Sleep( ms );
	}

	unsigned int f_waitforsingleobject( void* hHandle, unsigned int dwMilliseconds )
	{
		return WaitForSingleObject( static_cast< HANDLE >( hHandle ), dwMilliseconds );
	}


	/* class Mutex
	*/
	Mutex::Mutex()
	{
		m_mutexImpl = new CRITICAL_SECTION();
		InitializeCriticalSection( static_cast<CRITICAL_SECTION*>( m_mutexImpl ) );
	}

	Mutex::~Mutex()
	{
		DeleteCriticalSection( static_cast<CRITICAL_SECTION*>( m_mutexImpl ) );
		delete m_mutexImpl;
	}

	void Mutex::Lock()
	{
		EnterCriticalSection( static_cast<CRITICAL_SECTION*>( m_mutexImpl ) );
	}

	void Mutex::Unlock()
	{
		LeaveCriticalSection( static_cast<CRITICAL_SECTION*>( m_mutexImpl ) );
	}


	/* class PooledThread
	*/
	class PooledThread final
	{
		using CompletionCallback = std::function<void( ParallelWorker*, PooledThread* )>;

		// allow launch and go
		friend class ParallelWorker;
		friend void ParallelWorker_OnWorkCompleted( ParallelWorker* parallel, PooledThread* thread );

	public:
		explicit PooledThread( ParallelWorker* owner ) :
			m_owner( owner ),
			m_working( false ),
			m_dying( false )
		{
		}

		~PooledThread()
		{
			if ( Launched() )
			{
				std::terminate();
			}
		}

		FORCEINLINE bool Working() const
		{
			return m_working.Value();
		}

		FORCEINLINE bool Launched() const
		{
			return m_tdata.Handle != nullptr;
		}

	private:
		void Launch( CompletionCallback completionCallback, unsigned int stackSize = 0 )
		{
			if ( Launched() )
			{
				throw std::runtime_error( "already running thread" );
			}

			m_event.Create( true );
			m_callback = completionCallback;
			m_tdata.Handle = f_beginthread(
				nullptr,
				stackSize,
				[]( void* arg ) -> unsigned int		// thread func
				{
					if ( PooledThread* thread = reinterpret_cast< PooledThread* >( arg ) )
					{
						while ( !thread->m_dying )
						{
							if ( thread->m_tdata.Func )
							{
								thread->m_tdata.Func();								
								
								thread->m_working.Exchange( false );
								thread->m_event.Reset();

								if ( thread->m_callback )
								{
									thread->m_callback( thread->m_owner, thread );
								}
								thread->m_event.Wait();
							}
						}
						thread->m_dying.Exchange( false );
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
		}

		void Kill()
		{
			if ( !Launched() )
			{
				throw std::runtime_error( "not running thread" );
			}

			WaitTaskDone();

			m_dying.Exchange( true );			
			m_event.Signal();
			while ( m_dying )
			{
				Thread::Sleep( 0 );
			}
			ThreadData::MakeNull( m_tdata );
		}

		FORCEINLINE void WaitTaskDone()
		{
			// Wait until the assigned task is completed.
			while ( Working() );
		}

		FORCEINLINE void Go( const ThreadFunc& f )
		{
			WaitTaskDone();

			m_tdata.Func = f;
			m_working.Exchange( true );
			m_event.Signal();
		}

	private:
		ThreadData m_tdata;
		Event m_event;
		CompletionCallback m_callback;
		ParallelWorker* m_owner;
		Atomic<bool> m_working;
		Atomic<bool> m_dying;
	};


	/* class ParallelWorker
	*/
	ParallelWorker::ParallelWorker( size_t maxConcurrency )
	{
		m_pooledThreads.reserve( maxConcurrency );
		for ( size_t i = 0; i < maxConcurrency; ++i )
		{
			m_pooledThreads.emplace_back( std::make_shared<PooledThread>( this ) );
		}
	}

	ParallelWorker::~ParallelWorker()
	{
		WaitForAllTasks();
		
		for ( auto& thread : m_pooledThreads )
		{
			if ( thread->Launched() )
			{
				thread->Kill();
			}
		}
		m_pooledThreads.clear();
	}

	void ParallelWorker::Queue( ThreadFunc funcWork )
	{
		// Find an idle thread to work.
		auto thread = FindIdleThread();

		// If there is no idle thread, queue the task.
		if ( thread == nullptr )
		{
			LockGuard<Mutex> lockGuard( m_mutex );
			m_taskQueue.push( funcWork );
			return;
		}

		// Otherwise, go the task now.
		LockGuard<Mutex> lockGuard( m_mutex );
		thread->Go( funcWork );
	}

	void ParallelWorker::WaitForAllTasks()
	{
		for ( auto& thread : m_pooledThreads )
		{
			Assert( thread );
			while ( thread->Working() )
			{
				Thread::Sleep( 0 );
			}
		}
	}

	std::shared_ptr<PooledThread> ParallelWorker::FindIdleThread()
	{
		for ( auto& thread : m_pooledThreads )
		{
			Assert( thread );
			if ( thread->Working() )
			{
				continue;
			}

			if ( !thread->Launched() )
			{
				thread->Launch( &ParallelWorker_OnWorkCompleted );
			}
			return thread;
		}
		return nullptr;
	}

	void ParallelWorker_OnWorkCompleted( ParallelWorker* parallel, PooledThread* thread )
	{
		Assert( parallel );
		Assert( thread );

		//LockGuard<Mutex> lockGuard( parallel->m_mutex );	// ?? slow?
		if ( parallel->m_taskQueue.empty() )
		{
			return;
		}

		Thread::Sleep( 1 );

		// HACK: test once again due to concurrency
		LockGuard<Mutex> lockGuard( parallel->m_mutex );
		if ( !parallel->m_taskQueue.empty() && !thread->Working() )
		{
			thread->Go( parallel->m_taskQueue.front() );
			parallel->m_taskQueue.pop();
		}
	}
};



#include <iostream>


DEFINE_UNITTEST( atomic_test )
{
	Atomic<bool> atomBool( true );
	atomBool.Exchange( false );
	if ( !atomBool )
	{
		bool a = atomBool;
		std::cout << "<OK> atomic: " << atomBool << ", " << a << std::endl;
	}
	
	Atomic<short> atomShort;
	atomShort.Exchange( 66 );

	Atomic<int> atomInt;
	atomInt.Exchange( 3 );
	if ( atomInt == 3 )
	{
		int a = atomInt;
		std::cout << "<OK> atomic: " << atomInt << ", " << a << std::endl;
	}

	Atomic<__int64> atomInt64( 32 );
	++atomInt64;
	atomInt64.Exchange( 0xFFFFFFFFFFFF );
	if ( atomInt64 == 0xFFFFFFFFFFFF )
	{
		__int64 a = atomInt64;
		std::cout << "<OK> atomic: " << atomInt64 << ", " << a << std::endl;
	}

	//Atomic<float> atomShort;
	//Atomic<Event> atomClass;
}

DEFINE_UNITTEST( thread_test )
{
	float yCaptured = 13.2f;

	Thread t
	{
		[=]( int i, const char* str )
		{
			std::cout.sync_with_stdio();
			std::cout << "i: " << i << std::endl;
			std::cout << "str: " << str << std::endl;
			std::cout << "y : " << yCaptured << std::endl;
		},
		5,
		"test"
	};
	t.Join();
}

DEFINE_UNITTEST( parallel_test )
{
	std::cout.sync_with_stdio();

	ParallelWorker parallel( 3 );

	kih::ThreadFunc f0 = []() { std::cout << "f0" << std::endl;  };

	auto f1 = std::bind( []( int count ) { for ( int i = 0; i < count; ++i ); std::cout << "f1" << std::endl; }, 10000 );

	auto f2 = std::bind( []( const char* str ) { std::cout << "f2: " << str << std::endl; }, "parallel_bind" );

	std::vector<int> data{ 0, 1, 2, 3, 4, 5 };
	kih::ThreadFunc f3 = [=]() { std::cout << "f3: " << data[0] << std::endl; };

	auto f4 = std::bind( []( std::vector<int> data ) { std::cout << "f4: " << data[0] << std::endl; }, data );

	for ( int i = 0; i < 10; ++i )
	{
		parallel.Queue( f0 );
		parallel.Queue( f1 );
		parallel.Queue( f2 );
		parallel.Queue( f3 );
		parallel.Queue( f4 );
		
		std::cout << "(task " << i << "th\n" << std::endl;
	}

	Thread::Sleep( 100 );

	parallel.WaitForAllTasks();
}
