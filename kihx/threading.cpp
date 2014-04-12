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


	/* class Thread
	*/
	int Thread::HardwareConcurrency()
	{
		static int _HardwareConcurrency = 0;
		if ( _HardwareConcurrency == 0 )
		{
			SYSTEM_INFO sysInfo;
			GetSystemInfo( &sysInfo );
			_HardwareConcurrency = static_cast< int >( sysInfo.dwNumberOfProcessors );
		}
		return _HardwareConcurrency;
	}


	/* class TaskThread
	*/
	class TaskThread final
	{
		// allow launch and go
		friend class ThreadPool;

	public:
		explicit TaskThread( ThreadPool* owner ) :
			m_owner( owner ),
			m_dying( false )
		{
		}

		~TaskThread()
		{
			if ( Launched() )
			{
				std::terminate();
			}
		}

		FORCEINLINE bool Launched() const
		{
			return m_tdata.Handle != nullptr;
		}

		unsigned int Main()
		{
			Assert( m_owner );

			while ( !m_dying )
			{
				m_event.Wait();

				ThreadFunc task = m_tdata.Func;
				while ( task )
				{
					task();
					task = m_owner->FinishTaskAndGetNext( this );
				}
			}
			m_dying.Exchange( false );
			return 0;
		}

	private:
		void Launch( unsigned int stackSize = 0 )
		{
			if ( Launched() )
			{
				throw std::runtime_error( "already running thread" );
			}

			m_event.Create( false );

			m_tdata.Handle = f_beginthread(
				nullptr,
				stackSize,
				[]( void* arg ) -> unsigned int		// thread func
				{
					if ( TaskThread* thread = reinterpret_cast< TaskThread* >( arg ) )
					{
						return thread->Main();
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
		}

		void Kill()
		{
			if ( !Launched() )
			{
				return;
			}

			m_tdata.Func = nullptr;
			m_dying.Exchange( true );			
			m_event.Signal();
			f_waitforsingleobject( m_tdata.Handle, 0xFFFFFFFF );
			ThreadData::MakeNull( m_tdata );
		}

		FORCEINLINE void Go( ThreadFunc task )
		{
			if ( !Launched() )
			{
				Launch();
			}

			m_tdata.Func = task;
			m_event.Signal();
		}

	private:
		ThreadData m_tdata;
		Event m_event;
		ThreadPool* m_owner;
		Atomic<bool> m_dying;
	};


	/* class ParallelWorker
	*/
	ThreadPool::ThreadPool()
	{
		int concurrency = Thread::HardwareConcurrency();
		m_allThreads.reserve( concurrency );
		for ( int i = 0; i < concurrency; ++i )
		{
			m_allThreads.emplace_back( std::make_shared<TaskThread>( this ) );
			m_threadQueue.push( m_allThreads[i].get() );
		}
	}

	ThreadPool::~ThreadPool()
	{
		WaitForAllTasks();
		
		for ( auto& thread : m_allThreads )
		{
			if ( thread->Launched() )
			{
				thread->Kill();
			}
		}
		m_allThreads.clear();
	}

	void ThreadPool::Queue( ThreadFunc task )
	{
		if ( task == nullptr )
		{
			return;
		}

		LockGuard<Mutex> lockGuard( m_mutex );

		// If there is no idle thread, queue the task.
		if ( m_threadQueue.empty() )
		{
			m_taskQueue.push( task );
			return;
		}

		// Otherwise, go the task now.
		TaskThread* thread = m_threadQueue.front();
		m_threadQueue.pop();		
		Assert( thread );
		thread->Go( task );
	}

	void ThreadPool::WaitForAllTasks()
	{
		while ( m_threadQueue.size() != m_allThreads.size() )
		{
			Thread::Sleep( 0 );
		}
	}

	ThreadFunc ThreadPool::FinishTaskAndGetNext( TaskThread* thread )
	{
		ThreadFunc task = nullptr;

		LockGuard<Mutex> lockGuard( m_mutex );

		// Retrieve a queued task.
		if ( !m_taskQueue.empty() )
		{
			task = m_taskQueue.front();
			m_taskQueue.pop();
		}

		// If there is no task, push the thread into the idle thread pool.
		if ( task == nullptr )
		{
			m_threadQueue.push( thread );
		}

		return task;
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

	//Atomic<float> atomFloat;
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

DEFINE_UNITTEST( threadpool_test )
{
	std::cout.sync_with_stdio();

	ThreadPool* threadPool = ThreadPool::GetInstance();

	kih::ThreadFunc f0 = []() { std::cout << "f0" << std::endl;  };

	auto f1 = std::bind( []( int count ) { for ( int i = 0; i < count; ++i ); std::cout << "f1" << std::endl; }, 10000 );

	auto f2 = std::bind( []( const char* str ) { std::cout << "f2: " << str << std::endl; }, "parallel_bind" );

	std::vector<int> data{ 0, 1, 2, 3, 4, 5 };
	kih::ThreadFunc f3 = [=]() { std::cout << "f3: " << data[0] << std::endl; };

	auto f4 = std::bind( []( std::vector<int> data ) { std::cout << "f4: " << data[0] << std::endl; }, data );

	for ( int i = 0; i < 10; ++i )
	{
		threadPool->Queue( f0 );
		threadPool->Queue( f1 );
		threadPool->Queue( f2 );
		threadPool->Queue( f3 );
		threadPool->Queue( f4 );
		
		std::cout << "\n(task " << i << "th\n" << std::endl;
	}

	threadPool->WaitForAllTasks();

	std::cout << "\nthreadpool_test is done\n" << std::endl;
}
