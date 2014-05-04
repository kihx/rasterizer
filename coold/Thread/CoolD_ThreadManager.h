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
		ThreadManager();
		ThreadManager(const ThreadManager&) = delete;
		ThreadManager& operator=(const ThreadManager&) = delete;

	public:
		virtual	~ThreadManager(void);

	public:
		Dbool	Initialize(Dint threadCount = 0);
		Dvoid	Join();
		Dvoid	ClassificaionJoin(ThreadType type, bool isInfinite);		//특정타입에 대해서만 join
		HANDLE	Spawn(Duint(__stdcall *startAddress)(Dvoid*), Dvoid* parameter, Duint* threadID);			
		Dbool	WorkAllocation( ThreadWorkInfo info );

	private:
		mutex	m_containerSafeMutex;
		Duint	m_activeAbleThreadCount;
		vector<baseThread*> m_vecThreadPool;
		multimap<ThreadType, baseThread*> m_mapActiveThread;
	};
};

