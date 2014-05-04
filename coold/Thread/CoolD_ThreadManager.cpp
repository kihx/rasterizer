#include "CoolD_ThreadManager.h"
#include "..\Data\CoolD_Defines.h"
#include "..\data\CoolD_Inlines.h"
#include <process.h>

namespace CoolD
{
	ThreadManager::ThreadManager() 
		: m_activeAbleThreadCount( 1 )
	{
		Initialize();
	}

	ThreadManager::~ThreadManager()
	{
		Join();		
	}

	Dvoid ThreadManager::Join()
	{			
		for( auto iter = begin(m_mapActiveThread); iter != end(m_mapActiveThread); )
		{			
			baseThread* thread = iter->second;
			thread->JoinThread(true);
			m_vecThreadPool.push_back(thread);

			iter = m_mapActiveThread.erase(iter);
		}
	}

	HANDLE ThreadManager::Spawn(Duint(__stdcall *startAddress)(Dvoid*), Dvoid* parameter, Duint* threadID)
	{
		HANDLE threadHandle;
		threadHandle = (HANDLE)_beginthreadex(NULL, NULL, startAddress, parameter, 0, threadID);

		return threadHandle;
	}

	Dbool ThreadManager::Initialize(Dint threadCount)
	{
		lock_guard<mutex> lock(m_containerSafeMutex);
		Join();

		Safe_Delete_VecList(m_vecThreadPool);
		Safe_Delete_Map(m_mapActiveThread);

		if( threadCount == 0 )
		{
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			m_activeAbleThreadCount = sysinfo.dwNumberOfProcessors;
		}
		else
		{
			m_activeAbleThreadCount = threadCount;
		}
		
		for (Duint i = 0; i < m_activeAbleThreadCount; i++)
		{	//쓰레드 요소가 추가될 때 마다 여기에 생성 추가한다.
			baseThread* pWorkThread = new RenderThread();

			m_vecThreadPool.push_back( pWorkThread );
		}

		return true;
	}	

	Dbool ThreadManager::WorkAllocation( ThreadWorkInfo info )
	{				
		if (m_mapActiveThread.size() >= m_activeAbleThreadCount)	//지정한 스레드 갯수를 초과하면 다른 쓰레드가 완료될때까지 대기
		{
			ClassificaionJoin(info.type, false);
			return false;
		}

		lock_guard<mutex> lock(m_containerSafeMutex);
		auto findedThreadIter = find_if(begin(m_vecThreadPool), end(m_vecThreadPool), [ info ](const baseThread* thread){ return	thread->GetThreadType() == info.type; });

		if( findedThreadIter != end(m_vecThreadPool) )
		{
			baseThread* thread = (*findedThreadIter);
			if( thread->GetWorkInfo( info.pParam ) )
			{
				m_vecThreadPool.erase(findedThreadIter);
				m_mapActiveThread.insert({ info.type, thread });
				thread->Begin();
				return true;
			}								
		}	

		LOG("Param casting failed");
		return false;
	}
																	
	Dvoid ThreadManager::ClassificaionJoin(ThreadType type, bool isInfinite)
	{
		lock_guard<mutex> lock(m_containerSafeMutex);
		for (auto iter = begin(m_mapActiveThread); iter != end(m_mapActiveThread);)
		{
			if( iter->first == type )
			{
				baseThread* thread = iter->second;
				if (thread->JoinThread(isInfinite))
				{
					m_vecThreadPool.push_back(thread);
					iter = m_mapActiveThread.erase(iter);
				}				
			}
		}
	}
};