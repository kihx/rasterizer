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
		{	//���� ���� 0 �� ��� PC�� �⺻ �ھ�� ��ŭ ������ ���� ���� 
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

	//������ �����忡 �۾� �Ҵ�
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
	

	//������ ����ȭ�� ���ؼ� �۾��� �Ҵ�� ��� �������� �۾� ���Ḧ ��ٸ���
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
				{	//�۾����� �����尡 �ֱ� ������ ���
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