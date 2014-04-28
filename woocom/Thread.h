#pragma once

#include "common.h"
#include <vector>
#include <queue>
#include <functional>
#include <process.h>
#include <windows.h>

class WThreadPool;
class WWorker
{
public:
	WWorker(WThreadPool& pool, HANDLE event)
		:m_pool(pool), m_event(event), m_thread(nullptr), m_stop(0), m_threadID(0){}
	~WWorker();

	void operator =(const WWorker& rhs)
	{
		m_event = rhs.m_event;
		m_thread = rhs.m_thread;
		m_stop = rhs.m_stop;
		m_threadID = rhs.m_threadID;
	}

	// 스레드 생성 핸들 반환
	HANDLE Init();
	void Run();
	void Stop();
private:

	WThreadPool& m_pool;
	HANDLE m_event;
	HANDLE m_thread;
	volatile unsigned int m_stop;
	unsigned int m_threadID;
};

class WThreadPool
{
public:
	WThreadPool(size_t threadNum);
	~WThreadPool();

	void AddTask(const std::function<void()>& );
	void Join();

	size_t GetThreadNum() const;
private:
	friend class WWorker;

	CRITICAL_SECTION m_queue_mutex;

	std::queue< std::function<void()> > m_tasks;
	std::vector< HANDLE > m_threads;
	std::vector< WWorker* > m_workers;
	HANDLE m_event;
	volatile unsigned int m_numActiveThread;
};