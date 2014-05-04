#include "CoolD_Thread.h"
#include "CoolD_ThreadManager.h"
#include "..\Data\CoolD_Defines.h"

namespace CoolD
{
	baseThread::baseThread()
		: m_isStarted(false), m_threadType(ThreadType::BASE)
	{
	}

	baseThread::~baseThread()
	{
	}

	Dvoid baseThread::Begin()
	{
		if( m_isStarted )
			return;

		m_handleThread = GETSINGLE(ThreadManager).Spawn(baseThread::HandleRunner, this, &m_threadID);
		SetStarted(true);
	}	

	Duint __stdcall baseThread::HandleRunner(Dvoid* parameter)
	{
		baseThread* thread = (baseThread*)parameter;
		thread->Run();
		thread->SetStarted(false);

		return 0;
	}

	Dvoid baseThread::SetStarted(Dbool isStarted)
	{
		m_isStarted = isStarted;
	}

	Dbool baseThread::JoinThread(Dbool isInfinite)
	{
		return WaitForSingleObject(m_handleThread, (isInfinite) ? INFINITE : 0) == WAIT_OBJECT_0;
	}

	//----------------------------------------------------------------------------	
	RenderThread::RenderThread(void)
	{
		m_threadType = ThreadType::RENDER;
	}

	RenderThread::~RenderThread(void)
	{
		::WaitForSingleObject(m_handleThread, INFINITE);
	}

	void RenderThread::Run()
	{
		m_renderer.RenderBegin(m_pMesh);
		m_renderer.RenderEnd();
	}

	Dbool RenderThread::GetWorkInfo(InfoParam* pParam)
	{	//작업을 위해 필요한 내용 복사
		if( RenderInfoParam* renderInfoParam = dynamic_cast<RenderInfoParam*>(pParam) )
		{
			m_renderer = *renderInfoParam->renderer;
			m_pMesh = renderInfoParam->pMesh;
			return true;
		}				
		return false;
	}
}