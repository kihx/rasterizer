#include "stdafx.h"
#include "XtzThreadPool.h"
#include "Concommand.h"

#include <Windows.h>

namespace xtozero
{
	SpinLock CXtzThreadPool::m_lockObject;

	class CXtzThread
	{
	private:
		HANDLE m_thread;
		HANDLE m_threadEvent;
		volatile bool m_bEnd;

		WORK m_Work;

		static unsigned int WINAPI ThreadMain( LPVOID arg );

		CXtzThreadPool* m_pOwner;
		unsigned int m_index;
	public:
		explicit CXtzThread( CXtzThreadPool* pOwner, int index );
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
		unsigned int GetIndex()
		{
			return m_index;
		}
	};

	CXtzThread::CXtzThread( CXtzThreadPool* pOwner, int index ) : m_thread( nullptr ), m_threadEvent( nullptr ), m_pOwner( pOwner ), m_index( index )
	{
		m_threadEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
		m_thread = reinterpret_cast<HANDLE>(_beginthreadex( nullptr,
			0,
			ThreadMain,
			(LPVOID)this,
			0,
			nullptr ));
	}

	CXtzThread::~CXtzThread( )
	{
		if ( m_threadEvent )
		{
			CloseHandle( m_threadEvent );
		}
		if ( m_thread )
		{
			CloseHandle( m_thread );
		}
	}

	unsigned int WINAPI CXtzThread::ThreadMain( LPVOID arg )
	{
		CXtzThread* thread = reinterpret_cast<CXtzThread*>(arg);

		for ( ; ; )
		{
			thread->WaitEvent();
			if ( thread->IsEnd( ) )
			{
				return 0;
			}

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
		SYSTEM_INFO sysInfo;
		GetSystemInfo( &sysInfo );

		CreateThreadPool( sysInfo.dwNumberOfProcessors );
	}

	CXtzThreadPool::~CXtzThreadPool( )
	{
		DestroyThreadPool();
	}

	void CXtzThreadPool::CreateThreadPool( unsigned int maxThread )
	{
		if ( m_nThread == maxThread )
			return;

		unsigned int prevThread = m_nThread;
		m_nThread = maxThread;

		if ( m_threads.size() < maxThread )
		{
			for ( unsigned int i = m_threads.size( ); i < maxThread; ++i )
			{
				m_threads.push_back( std::make_unique<CXtzThread>( this, i ) );
				m_threadquere.push_back( m_threads[i].get( ) );
			}
		}
		else
		{
			if ( prevThread < m_nThread )
			{
				for ( unsigned int i = prevThread; i < maxThread; ++i )
				{
					m_threadquere.push_back( m_threads[i].get( ) );
				}
			}
		}
	}

	void CXtzThreadPool::DestroyThreadPool( )
	{
		for ( unsigned int i = 0; i < m_threads.size(); ++i )
		{
			CXtzThread* thread = m_threads[i].get();
			if ( thread != nullptr )
			{
				thread->SetEnd( true );
			}
		}

		WaitThread( );
	}

	void CXtzThreadPool::AddWork( WorkerFuntion worker, void* arg )
	{
		Lock<SpinLock> lock( m_lockObject );
		if ( m_threadquere.empty() )
		{
			m_workquere.emplace_back( worker, arg );
		}
		else
		{
			CXtzThread* thread = m_threadquere.front( );
			m_threadquere.pop_front( );
			WORK threadWork( worker, arg );
			thread->SetWork( threadWork );
			thread->WakeUp( );
		}
	}

	void CXtzThreadPool::AddThraed( CXtzThread* thread )
	{
		Lock<SpinLock> lock( m_lockObject );

		if ( m_nThread <= thread->GetIndex() )
		{
			return;
		}

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