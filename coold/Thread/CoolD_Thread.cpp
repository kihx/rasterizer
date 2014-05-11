#include "CoolD_Thread.h"
#include "CoolD_ThreadManager.h"
#include "..\Data\CoolD_Defines.h"
#include <process.h>

namespace CoolD
{
	RenderThread::RenderThread()
		: m_isFree(true), m_threadID(0), m_handleThread(0)
	{
		m_arrHandleWorkEvent[ WORK ] = CreateEvent(nullptr, true, false, nullptr);
		m_arrHandleWorkEvent[ SHUTDOWN ] = CreateEvent(nullptr, true, false, nullptr);
	}

	RenderThread::~RenderThread()
	{
		::WaitForSingleObject(m_handleThread, INFINITE);
	}

	HANDLE RenderThread::Begin()
	{
		m_handleThread = (HANDLE)_beginthreadex(NULL, NULL, RenderThread::HandleRunner, this, 0, &m_threadID);
		return m_handleThread;
	}	

	Duint __stdcall RenderThread::HandleRunner(Dvoid* parameter)
	{
		RenderThread* thread = (RenderThread*)parameter;
		Dbool isShutDown = false;

		while( !isShutDown )
		{			
			DWORD dwWaitResult = WaitForMultipleObjects(2, thread->m_arrHandleWorkEvent.data(), FALSE, INFINITE);

			if( dwWaitResult == WAIT_OBJECT_0 )
				thread->Run();
			else if( dwWaitResult == (WAIT_OBJECT_0 + 1) )
				isShutDown = true;			
		}		

		return 0;
	}

	Dvoid RenderThread::ReleaseHandles()
	{		
		CloseHandle(m_handleThread);
		CloseHandle(m_arrHandleWorkEvent[ 0 ]);
		CloseHandle(m_arrHandleWorkEvent[ 1 ]);
	}	
	
	//쓰레드 작업 함수
	Dvoid RenderThread::Run()
	{		
		m_renderer.RenderBegin(m_pMesh);
		m_renderer.RenderEnd();
					
		m_pMesh = nullptr;	
		SetIsFree(true);
		
		ResetEvent( m_arrHandleWorkEvent[0] );			
	}

	//작업진행 전에 필요한 인자들 복사
	Dvoid RenderThread::SetWorkInfo(RenderInfoParam* pParam)
	{
		if( pParam )
		{
			m_renderer = *pParam->renderer;
			m_pMesh = pParam->pMesh;
		}			
	}
	
	//이벤트 NonSignal -> Signal 변경 
	Dvoid RenderThread::MakeSignalEvent(EventType type)
	{
		SetEvent(m_arrHandleWorkEvent[type]);
	}

	//특정 이벤트가 Signal되었는지 대기 없이 곧바로 확인
	Dbool RenderThread::CheckEventSignal(EventType type)
	{
		return WaitForSingleObject(m_arrHandleWorkEvent[type], 0) == WAIT_OBJECT_0;
	}

	Dbool RenderThread::GetIsFree()
	{
		return m_isFree;
	}

	Dvoid RenderThread::SetIsFree(Dbool isFree)
	{
		m_isFree = isFree;
	}

	HANDLE RenderThread::GetThreadHandle() const
	{
		return m_handleThread;
	}

	

}