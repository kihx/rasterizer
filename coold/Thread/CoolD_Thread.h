#pragma once
#include "..\Data\CoolD_Type.h"
#include "..\Render\CoolD_RenderModule.h"

namespace CoolD
{
	class InfoParam
	{
	public:
		InfoParam() = default;
		virtual ~InfoParam(){};
	};

	class RenderInfoParam : public InfoParam
	{
	public:
		RenderModule* renderer;
		CustomMesh* pMesh;
		RenderInfoParam(RenderModule* _pRenderer, CustomMesh* _pMesh)
			:renderer(_pRenderer), pMesh(_pMesh)
		{
			
		}
	};

	struct ThreadWorkInfo
	{
		ThreadType type;
		InfoParam* pParam;

		ThreadWorkInfo(ThreadType _type, InfoParam* _pParam)
			:type(_type), pParam(_pParam)
		{}
	};

	class baseThread
	{
	public:
		baseThread();
		virtual	~baseThread();

	public:
		virtual Dvoid Run() = 0;		
		Dvoid Begin();
		Dvoid SetStarted(Dbool isStarted);
		Dbool JoinThread(Dbool isInfinite);
		virtual Dbool GetWorkInfo(InfoParam* pParam) = 0;
		inline ThreadType GetThreadType() const { return m_threadType; }

	private:
		static	Duint __stdcall HandleRunner( Dvoid* parameter );

	protected:
		HANDLE		m_handleThread;	
		ThreadType	m_threadType;
		Dbool		m_isStarted;
		Duint		m_threadID;
	};
		
	class RenderThread : public baseThread
	{	
		friend class ThreadManager;
	private:
		RenderThread(void);
		virtual ~RenderThread(void);

	public:
		virtual Dbool GetWorkInfo(InfoParam* pParam);
		virtual void Run();

	private:
		RenderModule m_renderer;
		CustomMesh* m_pMesh;
	};

	
};


