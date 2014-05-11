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

	public: //�ʱ�ȭ	�� ���� ����	
		Dbool	Initialize(Dint threadCount = 0);
		Dvoid	ResetThreads(Dint threadCount);
		Dvoid	CleanThreads();

	public: //������ �Ŵ����� ��Ȱ �� ���
		Dvoid	AssignWork(RenderInfoParam* info);
		Dvoid	WaitAllThreadWorking();

	private:
		vector<HANDLE>	m_vecThreadHandle;
		vector<RenderThread*> m_vecThreadPool;
		volatile unsigned int m_sContainerLock;		
	};
};

