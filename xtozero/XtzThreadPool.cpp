#include "stdafx.h"
#include "XtzThreadPool.h"

namespace xtozero
{
	HANDLE CXtzThreadPool::m_thread[MAX_THREAD];
	HANDLE CXtzThreadPool::m_threadEvent[MAX_THREAD];

	WORK CXtzThreadPool::m_work[MAX_THREAD];
	bool CXtzThreadPool::m_bWork[MAX_THREAD];

	CXtzThreadPool::CXtzThreadPool( ) : m_nThread( 0 )
	{
	}

	CXtzThreadPool::~CXtzThreadPool( )
	{
	}

	void CXtzThreadPool::CreateThreadPool( int maxThread )
	{
		InitializeCriticalSection( &cs );
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

	DWORD WINAPI CXtzThreadPool::ThreadFunction( LPVOID arg )
	{
		int threadIdx = (DWORD)arg;
		while ( true )
		{
			WaitForSingleObject( m_threadEvent[threadIdx], INFINITE );

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
		DeleteCriticalSection( &cs );
	}

	void CXtzThreadPool::AddWork( WorkerFuntion worker, LPVOID arg )
	{
		EnterCriticalSection( &cs );

		m_workquere.emplace_back( worker, arg );

		LeaveCriticalSection( &cs );
	}

	void CXtzThreadPool::Run( void )
	{
		while ( true )
		{
			EnterCriticalSection( &cs );

			if ( m_workquere.size() > 0 )
			{
				for ( int i = 0; i < m_nThread; ++i )
				{
					if ( m_bWork[i] )
					{

					}
					else
					{
						WORK& work = m_workquere.front();
						m_work[i].m_worker = work.m_worker;
						m_work[i].m_arg = work.m_arg;
						m_bWork[i] = true;
						m_workquere.pop_front();
						SetEvent( m_threadEvent[i] );
						break;
					}
				}
			}
			else
			{
				break;
			}

			LeaveCriticalSection( &cs );
		}
	}
}	