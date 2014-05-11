#pragma once
#include <Windows.h>
#include "..\Data\CoolD_Type.h"
#include "..\Data\CoolD_Singleton.h"
#include "..\Data\CoolD_Struct.h"
#include "CoolD_Thread.h"

namespace CoolD
{
	class baseThread;
	class ThreadManager : public CSingleton<ThreadManager>
	{
		friend class CSingleton<ThreadManager>;	
	private:
		ThreadManager(const ThreadManager&) = delete;
		ThreadManager& operator=(const ThreadManager&) = delete;

	public:
		ThreadManager();
		virtual	~ThreadManager(void);

	public: //초기화	및 정리 관련	
		Dbool	Initialize(Dint threadCount = 0);
		Dvoid	ResetThreads(Dint threadCount);
		Dvoid	CleanThreads();

	public: //쓰레드 매니저의 역활 및 기능
		Dvoid	AssignWork(RenderInfoParam* info);
		Dvoid	WaitAllThreadWorking();

	private:
		vector<HANDLE>	m_vecThreadHandle;
		vector<RenderThread*> m_vecThreadPool;
		volatile unsigned int m_sContainerLock;		
	};
};

