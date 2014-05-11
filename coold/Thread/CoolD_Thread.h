#pragma once
#include "..\Data\CoolD_Type.h"
#include "..\Render\CoolD_RenderModule.h"
#include <windows.h>
namespace CoolD
{
#define  ENTER_LFS_LOCK_(lock)   while( InterlockedExchange( lock, 1 ) == 1 ){ Sleep( 1 ); }
#define  EXIT_LFS_LOCK_(lock)    InterlockedExchange( lock, 0 );

	class Lock
	{
	
	private:
		volatile unsigned int* m_sLock;

	public:
		Lock(volatile unsigned int* slock)
		{
			m_sLock = slock;
			ENTER_LFS_LOCK_(m_sLock)
		}

		~Lock()
		{
			EXIT_LFS_LOCK_(m_sLock)
		}
	};
	
	class RenderInfoParam
	{
	public:
		RenderModule* renderer;
		CustomMesh* pMesh;
		RenderInfoParam(RenderModule* _pRenderer, CustomMesh* _pMesh)
			:renderer(_pRenderer), pMesh(_pMesh)
		{}
	};	

	class RenderThread
	{
	public:
		RenderThread();
		~RenderThread();
		
	public:
		Dvoid	Run();
		HANDLE	Begin();

		Dvoid	MakeSignalEvent(EventType type);		
		Dbool	CheckEventSignal(EventType type);

		Dvoid	ReleaseHandles();
		HANDLE	GetThreadHandle() const;
		Dvoid	SetIsFree(Dbool isFree);
		Dbool	GetIsFree();
		Dvoid	SetWorkInfo(RenderInfoParam* pParam);

	private:
		static	Duint __stdcall HandleRunner( Dvoid* parameter );
				
	protected:
		HANDLE	m_handleThread;	
		Dbool	m_isFree;	
		Duint	m_threadID;
		array<HANDLE, TYPECOUNT> m_arrHandleWorkEvent;

	private:
		RenderModule	m_renderer;
		CustomMesh*		m_pMesh;		
	};
};


