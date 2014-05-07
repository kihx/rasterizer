#include "stdafx.h"
#include "threading.h"
#include "concommand.h"
#include "profiler.h"

#include <process.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


namespace kih
{
	char f_interlockedexchange8( volatile char* target, char value )
	{
		return InterlockedExchange8( target, value );
	}

	short f_interlockedexchange16( volatile short* target, short value )
	{
		return InterlockedExchange16( target, value );
	}

	int f_interlockedexchange32( volatile int* target, int value )
	{
		return InterlockedExchange( ( volatile long* ) target, value );
	}

	int64 f_interlockedexchange64( volatile int64* target, int64 value )
	{
		return InterlockedExchange64( target, value );
	}
	
	short f_interlockedincrement16( volatile short* target )
	{
		return InterlockedIncrement16( target );
	}

	int f_interlockedincrement32( volatile int* target )
	{
		return InterlockedIncrement( ( volatile long* ) target );
	}

	int64 f_interlockedincrement64( volatile int64* target )
	{
		return InterlockedIncrement64( target );
	}

	short f_interlockeddecrement16( volatile short* target )
	{
		return InterlockedDecrement16( target );
	}

	int f_interlockeddecrement32( volatile int* target )
	{
		return InterlockedDecrement( ( volatile long* ) target );
	}

	int64 f_interlockeddecrement64( volatile int64* target )
	{
		return InterlockedDecrement64( target );
	}

	int f_interlockedadd32( volatile int* target, int value )
	{
		return InterlockedAdd( ( volatile long* ) target, value );
	}

	int64 f_interlockedadd64( volatile int64* target, int64 value )
	{
		return InterlockedAdd64( target, value );
	}

	short f_interlockedcompareexchange16( volatile short* destination, short exchange, short comparand )
	{
		return InterlockedCompareExchange16( destination, exchange, comparand );
	}

	int f_interlockedcompareexchange32( volatile int* destination, int exchange, int comparand )
	{
		return InterlockedCompareExchange( ( volatile long* ) destination, exchange, comparand );
	}

	int64 f_interlockedcompareexchange64( volatile int64* destination, int64 exchange, int64 comparand )
	{
		return InterlockedCompareExchange64( destination, exchange, comparand );
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


	/* class SpinLock
	*/
	void SpinLock::Lock()
	{
		while ( true )
		{
			if ( m_atom.CompareExchange( 1, 0 ) == 0 )
			{
				break;
			}
		}
	}

	void SpinLock::Unlock()
	{
		m_atom.Exchange( 0 );
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
	bool Thread::IsInMainThread()
	{
		static unsigned int s_MainThreadID = GetCurrentThreadId();
		return GetCurrentThreadId() == s_MainThreadID;
	}

	unsigned int Thread::CurrentThreadID()
	{
		static thread_local unsigned int tls_ThreadID = 0;
		if ( tls_ThreadID == 0 )
		{
			tls_ThreadID = GetCurrentThreadId();
		}
		return tls_ThreadID;
	}

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

			Thread::CurrentThreadID();	// Initialize the TLS thread ID.

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
		m_threadPool.reserve( concurrency );
		for ( int i = 0; i < concurrency; ++i )
		{
			m_threadPool.emplace_back( std::make_shared<TaskThread>( this ) );
			m_threadQueue.push( m_threadPool[i].get() );
		}
	}

	ThreadPool::~ThreadPool()
	{
		WaitForAllTasks();
		
		for ( auto& thread : m_threadPool )
		{
			if ( thread->Launched() )
			{
				thread->Kill();
			}
		}
		m_threadPool.clear();
	}

	void ThreadPool::Queue( ThreadFunc task )
	{
		if ( task == nullptr )
		{
			return;
		}

		// local scope
		TaskThread* thread = nullptr;
		{
			LockGuard<Mutex> lockGuard( m_mutex );

			// If there is no idle thread, queue the task.
			if ( m_threadQueue.empty() )
			{
				m_taskQueue.push( task );
				return;
			}

			// Otherwise, go the task now.
			thread = m_threadQueue.front();
			m_threadQueue.pop();
		}

		if ( thread )
		{
			thread->Go( task );
		}
	}

	void ThreadPool::WaitForAllTasks()
	{
		while ( m_threadQueue.size() != m_threadPool.size() )
		{
			Thread::Sleep( 1 );
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

	Atomic<int64> atomInt64( 32 );
	++atomInt64;
	atomInt64.Exchange( 0xFFFFFFFFFFFF );
	if ( atomInt64 == 0xFFFFFFFFFFFF )
	{
		int64 a = atomInt64;
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

	kih::ThreadFunc f0 = []() { std::cout << "f0" << Thread::CurrentThreadID() << std::endl;  };

	auto f1 = std::bind( []( int count ) { for ( int i = 0; i < count; ++i ); std::cout << "f1" << Thread::CurrentThreadID() << std::endl; }, 10000 );

	auto f2 = std::bind( []( const char* str ) { std::cout << "f2: " << str << Thread::CurrentThreadID() << std::endl; }, "parallel_bind" );

	StlVector<int> data{ 0, 1, 2, 3, 4, 5 };
	kih::ThreadFunc f3 = [=]() { std::cout << "f3: " << data[0] << Thread::CurrentThreadID() << std::endl; };

	auto f4 = std::bind( []( StlVector<int> data ) { std::cout << "f4: " << data[0] << Thread::CurrentThreadID() << std::endl; }, data );

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
