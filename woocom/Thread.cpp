#include "thread.h"

#include <iostream>

WWorker::~WWorker()
{
	CloseHandle(m_thread);
}

HANDLE WWorker::Init()
{
	m_thread = (HANDLE)_beginthreadex(
		nullptr,
		0,
		[](void* arg) -> unsigned
	{
		WWorker* worker = reinterpret_cast<WWorker*>(arg);
		if (worker)
		{
			worker->Run();
		}
		return 0;
	},
		this,
		0,
		&m_threadID
		);

	if (m_thread == nullptr)
	{
		std::cout << "_beginthreadex failed." << std::endl;
		return nullptr;
	}

	return m_thread;
}

void WWorker::Run()
{
	while (true)
	{
		std::function<void()> task = nullptr;
		{
			WMutex mutex(&m_pool.m_queue_mutex);

			if (!m_pool.m_tasks.empty())
			{
				task = m_pool.m_tasks.front();
				m_pool.m_tasks.pop();
				_InterlockedIncrement(&m_pool.m_numActiveThread);
			}
		}

		if (task)
		{
			task();
		}
		else
		{
			if (m_pool.m_stop)
			{
				return;
			}

			_InterlockedDecrement(&m_pool.m_numActiveThread);
			WaitForSingleObject(m_event, INFINITE);
		}
	}
}

WThreadPool::WThreadPool(size_t threadNum) : m_stop(false)
{
	InitializeCriticalSection(&m_queue_mutex);

	m_event = CreateEvent(nullptr, TRUE, FALSE, L"wakeup_event");
	m_threads.reserve(threadNum);

	for (size_t i = 0; i < threadNum; ++i)
	{
		WWorker* worker = new WWorker(*this, m_event);

		HANDLE hThread = worker->Init();
		m_threads.push_back(hThread);
		m_workers.push_back(worker);
	}
}

WThreadPool::~WThreadPool()
{
	{
		WMutex mutex(&m_queue_mutex);
		m_stop = true;
	}

	for (size_t i = 0; i < m_threads.size(); ++i)
	{
		delete m_workers[i];
	}

	CloseHandle(m_event);
	DeleteCriticalSection(&m_queue_mutex);
}

void WThreadPool::AddTask(std::function<void()> task)
{
	{
		WMutex mutex(&m_queue_mutex);
		m_tasks.push(task);
	}

	SetEvent(m_event);
}

void WThreadPool::Join()
{
	while ( m_numActiveThread == 0)
	{
		Sleep(1);
	}
}