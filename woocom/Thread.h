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

	// ������ ���� �ڵ� ��ȯ
	HANDLE Init();
	void Run();
	void Stop();
	void Signal();
private:

	WThreadPool& m_pool;
	HANDLE m_event;
	HANDLE m_thread;
	HANDLE m_exitEvent;
	volatile unsigned int m_stop;
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
	volatile unsigned int m_numActiveThread;
};