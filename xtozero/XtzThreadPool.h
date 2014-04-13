#ifndef _XTZTHREADPOOL_H_
#define _XTZTHREADPOOL_H_

#include <list>
#include <Windows.h>

namespace xtozero
{
	const int MAX_THREAD = 16;

	typedef void( *WorkerFuntion )(LPVOID);

	struct WORK
	{
		WORK() {}
		WORK( WorkerFuntion worker, LPVOID arg )
		: m_worker( worker ), m_arg( arg )
		{}

		WorkerFuntion	m_worker;
		LPVOID			m_arg;
	};

	class CXtzThreadPool
	{
	private:
		static CRITICAL_SECTION m_cs;

		int m_nThread;

		static HANDLE m_thread[MAX_THREAD];
		static HANDLE m_threadEvent[MAX_THREAD];
		static bool m_bWork[MAX_THREAD];

		static WORK m_work[MAX_THREAD];

		static std::list<WORK> m_workquere;

	public:
		CXtzThreadPool( );
		~CXtzThreadPool();

		void CreateThreadPool(int maxThread);
		void DestroyThreadPool();
		static DWORD WINAPI ThreadFunction(LPVOID arg);
		void AddWork( WorkerFuntion worker, LPVOID arg );
		void Run( void );
	};
}

#endif