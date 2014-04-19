#include "stdafx.h"
#include "XtzThreadPool.h"

namespace xtozero
{
	CriticalSection CXtzThreadPool::m_cs;

	class CXtzThread
	{
	private:
		HANDLE m_thread;
		HANDLE m_threadEvent;
		bool m_bEnd;

		WORK m_Work;

		static DWORD WINAPI ThreadMain( LPVOID arg );

		CXtzThreadPool* m_pOwner;
	public:
		explicit CXtzThread( CXtzThreadPool* pOwner );
		~CXtzThread( );

		void WaitEvent( );
		void Work( );
		void WakeUp( );
		bool IsEnd( ) { return m_bEnd; }
		CXtzThreadPool* GetOwner() { return m_pOwner; }
		void SetEnd( bool end ) { m_bEnd = end; }
		void SetWork( WORK& work )
		{
			m_Work = work;
		}
	};

	CXtzThread::CXtzThread( CXtzThreadPool* pOwner ) : m_thread( nullptr ), m_threadEvent( nullptr ), m_pOwner( pOwner )
	{
		m_threadEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
		m_thread = CreateThread( nullptr,
			0,
			ThreadMain,
			(LPVOID)this,
			0,
			nullptr );
	}

	CXtzThread::~CXtzThread( )
	{
		CloseHandle( m_threadEvent );
		CloseHandle( m_thread );
	}

	DWORD WINAPI CXtzThread::ThreadMain( LPVOID arg )
	{
		CXtzThread* thread = reinterpret_cast<CXtzThread*>(arg);

		while ( true )
		{
			if ( thread->IsEnd( ) )
			{
				break;
			}

			thread->WaitEvent();
			
			thread->Work( );
			thread->GetOwner()->AddThraed( thread );
		}

		return 0;
	}

	void CXtzThread::WaitEvent()
	{
		WaitForSingleObject( m_threadEvent, INFINITE );
	}

	void CXtzThread::Work()
	{
		assert( m_Work.m_worker );
		m_Work.m_worker( m_Work.m_arg );
	}

	void CXtzThread::WakeUp()
	{
		SetEvent( m_threadEvent );
	}

	CXtzThreadPool::CXtzThreadPool( ) : m_nThread( 0 )
	{
	}

	CXtzThreadPool::~CXtzThreadPool( )
	{
	}

	void CXtzThreadPool::CreateThreadPool( int maxThread )
	{
		if ( m_nThread > 0 )
			return;

		m_nThread = maxThread;

		for ( int i = 0; i < m_nThread; ++i )
		{
			m_threads.push_back( std::make_unique<CXtzThread>( this ) );
			m_threadquere.push_back( m_threads[i].get() );
		}
	}

	void CXtzThreadPool::DestroyThreadPool( )
	{
		for ( int i = 0; i < MAX_THREAD; ++i )
		{
			CXtzThread* thread = m_threads[i].get();
			thread->SetEnd( true );
			thread->WakeUp( );
		}

		WaitThread();
	}

	void CXtzThreadPool::AddWork( WorkerFuntion worker, void* arg )
	{
		Lock<CriticalSection> lock( m_cs );

		if ( m_threadquere.empty() )
		{
			m_workquere.emplace_back( worker, arg );
		}
		else
		{
			CXtzThread* thread = m_threadquere.front( );
			m_threadquere.pop_front( );
			thread->SetWork( WORK( worker, arg ) );
			thread->WakeUp( );
		}
	}

	void CXtzThreadPool::AddThraed( CXtzThread* thread )
	{
		Lock<CriticalSection> lock( m_cs );

		if ( m_workquere.empty() )
		{
			m_threadquere.push_back( thread );
		}
		else
		{
			WORK work = m_workquere.front( );
			m_workquere.pop_front( );
			thread->SetWork( work );
			thread->WakeUp( );
		}
	}

	void CXtzThreadPool::WaitThread( )
	{
		while ( m_threadquere.size( ) != m_nThread )
		{
			Sleep( 0 );
		}
	}
}	