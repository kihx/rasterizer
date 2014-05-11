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
	while ( m_stop == 0 )
	{
		std::function<void()> task = nullptr;
		{
			WMutex mutex(&m_pool.m_queue_mutex);

			if (!m_pool.m_tasks.empty())
			{
				_InterlockedIncrement(&m_pool.m_numActiveThread);

				task = m_pool.m_tasks.front();
				m_pool.m_tasks.pop();

			}
		}

		if (task)
		{
			task();

			_InterlockedDecrement(&m_pool.m_numActiveThread);
		}
		else
		{
			WaitForSingleObject(m_event, INFINITE);
		}
	}
}

void WWorker::Stop()
{
	InterlockedExchange(&m_stop, 1);
	SetEvent(m_event);
}

WThreadPool::WThreadPool(size_t threadNum) : m_numActiveThread(0)
{
	InitializeCriticalSection(&m_queue_mutex);

	m_event = CreateEvent(nullptr, FALSE, FALSE, L"wakeup_event");
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
	for (size_t i = 0; i < m_threads.size(); ++i)
	{
		m_workers[i]->Stop();
		delete m_workers[i];
	}

	CloseHandle(m_event);
	DeleteCriticalSection(&m_queue_mutex);
}

void WThreadPool::AddTask(const std::function<void()>& task)
{
	WMutex mutex(&m_queue_mutex);
	m_tasks.push(task);

	SetEvent(m_event);
}

void WThreadPool::Join()
{
	while (m_numActiveThread != 0 || !m_tasks.empty())
	{
		Sleep(0);
	}
}

size_t WThreadPool::GetThreadNum() const
{
	return m_threads.size();
}