#include "stdafx.h"
#include "XtzThreadPool.h"

namespace xtozero
{
	HANDLE CXtzThreadPool::m_thread[MAX_THREAD];
	HANDLE CXtzThreadPool::m_threadEvent[MAX_THREAD];

	LPVOID CXtzThreadPool::m_threadArg[MAX_THREAD];
	WorkerFuntion CXtzThreadPool::m_worker[MAX_THREAD];

	CXtzThreadPool::CXtzThreadPool( ) : m_nThread( 0 )
	{
	}

	CXtzThreadPool::~CXtzThreadPool( )
	{
	}

	void CXtzThreadPool::CreateThreadPool( int maxThread )
	{
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
		}
	}

	DWORD WINAPI CXtzThreadPool::ThreadFunction( LPVOID arg )
	{
		while ( true )
		{
			WaitForSingleObject( m_threadEvent[(DWORD)arg], INFINITE );

			m_worker[(DWORD)arg]( m_threadArg[(DWORD)arg] );
		}
	}

	void CXtzThreadPool::DestroyThreadPool( )
	{
		for ( int i = 0; i < MAX_THREAD; ++i )
		{
			CloseHandle( m_threadEvent[i] );
			CloseHandle( m_thread[i] );
		}
	}
}	