#ifndef _XTZTHREADPOOL_H_
#define _XTZTHREADPOOL_H_

#include <list>
#include <vector>
#include <memory>
#include <Windows.h>

namespace xtozero
{
	const int MAX_THREAD = 8;

	typedef void( *WorkerFuntion )(LPVOID);

	class CriticalSection
	{
	private:
		CRITICAL_SECTION m_cs;

	public:
		explicit CriticalSection()
		{
			InitializeCriticalSection( &m_cs );
		}
		
		~CriticalSection()
		{
			DeleteCriticalSection( &m_cs );
		}

		CriticalSection( const CriticalSection& cs ) : m_cs( cs.m_cs )
		{

		}

		void Lock()
		{
			EnterCriticalSection( &m_cs );
		}

		void Unlock()
		{
			LeaveCriticalSection( &m_cs );
		}
	};

	template <typename T>
	class Lock
	{
	private:
		T& m_lock;

		Lock& operator=(const T& lock);
		Lock( const T& lock );
	public:
		explicit Lock( T& lock ) : m_lock( lock )
		{
			m_lock.Lock( );
		}
		~Lock( )
		{
			m_lock.Unlock( );
		}
	};

	struct WORK
	{
		WORK( ) : m_worker( nullptr ), m_arg( nullptr )
		{}
		WORK( WorkerFuntion worker, LPVOID arg )
		: m_worker( worker ), m_arg( arg )
		{}

		WorkerFuntion	m_worker;
		LPVOID			m_arg;
	};

	class CXtzThread;

	class CXtzThreadPool
	{
	private:
		int m_nThread;

		static CriticalSection m_cs;
		std::list<WORK> m_workquere;
		std::list<CXtzThread*> m_threadquere;
		std::vector<std::unique_ptr<CXtzThread>> m_threads;
	public:
		CXtzThreadPool( );
		~CXtzThreadPool();

		std::list<WORK>& GetWorkQuere() { return m_workquere; }

		void CreateThreadPool(int maxThread);
		void DestroyThreadPool();
		void AddWork( WorkerFuntion worker, LPVOID arg );
		void AddThraed( CXtzThread* thread );
		void WaitThread();
	};
}

#endif