#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <process.h>
#include <windows.h>

class WMutex
{
public:
	WMutex(CRITICAL_SECTION* cs) : m_cs(cs)
	{
		EnterCriticalSection(m_cs);
	}
	~WMutex()
	{
		LeaveCriticalSection(m_cs);
	}
private:
	CRITICAL_SECTION*	m_cs;
};

class WThreadPool;
class WWorker
{
public:
	WWorker(WThreadPool& pool, HANDLE event)
		:m_pool(pool), m_event(event), m_thread(nullptr), m_threadID(0){}
	~WWorker();

	// 스레드 생성 핸들 반환
	HANDLE Init();
	void Run();
private:

	WThreadPool& m_pool;
	HANDLE m_event;
	HANDLE m_thread;
	unsigned int m_threadID;
};

class WThreadPool
{
public:
	WThreadPool(size_t threadNum);
	~WThreadPool();

	void AddTask(std::function<void()>);
	void Join();
private:
	friend class WWorker;

	CRITICAL_SECTION m_queue_mutex;

	std::queue< std::function<void()> > m_tasks;
	std::vector< HANDLE > m_threads;
	std::vector< WWorker* > m_workers;
	HANDLE m_event;
	bool m_stop;
	volatile unsigned int m_numActiveThread;
};