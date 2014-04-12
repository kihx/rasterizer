#include "stdafx.h"
#include "XtzThreadPool.h"

namespace xtozero
{
	CRITICAL_SECTION CXtzThreadPool::m_cs;

	HANDLE CXtzThreadPool::m_thread[MAX_THREAD];
	HANDLE CXtzThreadPool::m_threadEvent[MAX_THREAD];

	WORK CXtzThreadPool::m_work[MAX_THREAD];
	bool CXtzThreadPool::m_bWork[MAX_THREAD];
	std::list<WORK> CXtzThreadPool::m_workquere;

	CXtzThreadPool::CXtzThreadPool( ) : m_nThread( 0 )
	{
	}

	CXtzThreadPool::~CXtzThreadPool( )
	{
		for ( int i = 0; i < m_nThread; ++i )
		{
			TerminateThread( m_thread[i], 0 );
		}
		DestroyThreadPool();
	}

	void CXtzThreadPool::CreateThreadPool( int maxThread )
	{
		if ( m_nThread > 0 )
			return;

		InitializeCriticalSection( &m_cs );
		m_nThread = maxThread;

		for ( int i = 0; i < MAX_THREAD; ++i )
		{
			m_threadEvent[i] = CreateEvent( nullptr, FALSE, FALSE, nullptr );
			m_thread[i] = CreateThread( nullptr,
				0,
				ThreadFunction,
				(LPVOID)i,
				0,
				nullptr );
			m_bWork[i] = false;
			m_work[i].m_worker = nullptr;
		}
	}

	DWORD WINAPI CXtzThreadPool::ThreadFunction( void* arg )
	{
		int threadIdx = (DWORD)arg;
		while ( true )
		{
			WaitForSingleObject( m_threadEvent[threadIdx], INFINITE );
			m_bWork[threadIdx] = true;

			EnterCriticalSection( &m_cs );
			WORK work;
			if ( m_workquere.size() > 0 )
			{
				work = m_workquere.front();
				m_workquere.pop_front();
			}
			else
			{
				LeaveCriticalSection( &m_cs );
				m_bWork[threadIdx] = false;
				continue;
			}
			LeaveCriticalSection( &m_cs );
			m_work[threadIdx].m_worker = work.m_worker;
			m_work[threadIdx].m_arg = work.m_arg;
			
			assert( m_work[threadIdx].m_worker );
			if ( m_work[threadIdx].m_worker )
			{
				LPVOID& arg = m_work[threadIdx].m_arg;
				m_work[threadIdx].m_worker( arg );
			}

			m_bWork[threadIdx] = false;
		}
	}

	void CXtzThreadPool::DestroyThreadPool( )
	{
		for ( int i = 0; i < MAX_THREAD; ++i )
		{
			CloseHandle( m_threadEvent[i] );
			CloseHandle( m_thread[i] );
		}
		DeleteCriticalSection( &m_cs );
	}

	void CXtzThreadPool::AddWork( WorkerFuntion worker, void* arg )
	{
		EnterCriticalSection( &m_cs );

		m_workquere.emplace_back( worker, arg );

		LeaveCriticalSection( &m_cs );
	}

	void CXtzThreadPool::Run( void )
	{
		while ( true )
		{
			if ( m_workquere.size() > 0 )
			{
				for ( int i = 0; i < m_nThread; ++i )
				{
					if ( m_bWork[i] )
					{

					}
					else
					{
						SetEvent( m_threadEvent[i] );
						break;
					}
				}
			}
			else
			{
				return;
			}
			Sleep( 0 );
		}
	}
}	