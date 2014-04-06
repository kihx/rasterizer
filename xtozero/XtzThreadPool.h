#ifndef _XTZTHREADPOOL_H_
#define _XTZTHREADPOOL_H_

namespace xtozero
{
	const int MAX_THREAD = 8;

	typedef void( *WorkerFuntion )(LPVOID);

	class CXtzThreadPool
	{
	private:
		CXtzThreadPool();

		int m_nThread;

		static HANDLE m_thread[MAX_THREAD];
		static HANDLE m_threadEvent[MAX_THREAD];

		static LPVOID m_threadArg[MAX_THREAD];
		static WorkerFuntion m_worker[MAX_THREAD];

	public:
		~CXtzThreadPool();

		void CreateThreadPool(int maxThread);
		void DestroyThreadPool();
		static DWORD WINAPI ThreadFunction(LPVOID arg);
	};
}

#endif