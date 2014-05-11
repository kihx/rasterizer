#include "CoolD_ThreadManager.h"
#include "..\Data\CoolD_Defines.h"
#include "..\data\CoolD_Inlines.h"


namespace CoolD
{
	ThreadManager::ThreadManager() 
		: m_sContainerLock(0)
	{
		Initialize();
	}

	ThreadManager::~ThreadManager()
	{			
		CleanThreads();
	}

	Dbool ThreadManager::Initialize(Dint threadCount)
	{	
		Lock lock(&m_sContainerLock);
		if( threadCount == 0 )
		{	//전달 값이 0 일 경우 PC의 기본 코어수 만큼 쓰레드 갯수 설정 
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			threadCount = sysinfo.dwNumberOfProcessors;
		}		
		
		for( Dint i = 0; i < threadCount; i++ )
		{	
			m_vecThreadPool.push_back( new RenderThread() );
			m_vecThreadHandle.push_back( m_vecThreadPool[ i ]->Begin() );
		}

		return true;
	}

	//각각의 쓰레드에 작업 할당
	Dvoid ThreadManager::AssignWork(RenderInfoParam* info)
	{
		Lock lock(&m_sContainerLock);
		for( ; ; )
		{			
			for( auto thread : m_vecThreadPool )
			{
				if( thread->GetIsFree() == true )
				{
					thread->SetIsFree(false);
					thread->SetWorkInfo(info);
					thread->MakeSignalEvent(WORK);
					return;
				}
			}
		}
	}	
	

	//프레임 동기화를 위해서 작업이 할당된 모든 쓰레드의 작업 종료를 기다린다
	Dvoid ThreadManager::WaitAllThreadWorking()
	{
		Lock lock(&m_sContainerLock);
		Dbool isLoop = true;

		for ( ; isLoop ; )
		{
			isLoop = false;
			for( auto thread : m_vecThreadPool )
			{
				if( thread->CheckEventSignal(WORK) )
				{	//작업중인 쓰레드가 있기 때문에 대기
					isLoop = true;
					break;
				}				
			}
		}
	}

	Dvoid ThreadManager::ResetThreads(Dint threadCount)
	{
		CleanThreads();
		Initialize(threadCount);
	}

	Dvoid ThreadManager::CleanThreads()
	{
		Lock lock(&m_sContainerLock);

		if( m_vecThreadPool.empty() )
			return;

		for( auto thread : m_vecThreadPool )
		{
			thread->MakeSignalEvent(SHUTDOWN);
		}

		WaitForMultipleObjects(m_vecThreadHandle.size(), m_vecThreadHandle.data(), TRUE, INFINITE);
		
		for( auto thread : m_vecThreadPool )
		{
			thread->ReleaseHandles();
		}

		Safe_Delete_VecList(m_vecThreadPool);
		m_vecThreadHandle.clear();				
	}
};