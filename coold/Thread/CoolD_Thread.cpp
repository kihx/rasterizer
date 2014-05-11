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
	
	//������ �۾� �Լ�
	Dvoid RenderThread::Run()
	{		
		m_renderer.RenderBegin(m_pMesh);
		m_renderer.RenderEnd();
					
		m_pMesh = nullptr;	
		SetIsFree(true);
		
		ResetEvent( m_arrHandleWorkEvent[0] );			
	}

	//�۾����� ���� �ʿ��� ���ڵ� ����
	Dvoid RenderThread::SetWorkInfo(RenderInfoParam* pParam)
	{
		if( pParam )
		{
			m_renderer = *pParam->renderer;
			m_pMesh = pParam->pMesh;
		}			
	}
	
	//�̺�Ʈ NonSignal -> Signal ���� 
	Dvoid RenderThread::MakeSignalEvent(EventType type)
	{
		SetEvent(m_arrHandleWorkEvent[type]);
	}

	//Ư�� �̺�Ʈ�� Signal�Ǿ����� ��� ���� ��ٷ� Ȯ��
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