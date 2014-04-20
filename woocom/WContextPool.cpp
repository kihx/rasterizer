#include "WModule.h"
#include "common.h"
#include "WContext.h"
#include "WContextPool.h"

WContextPool::WContextPool(int contextNum, WModule* module)
{
	if (module == nullptr)
	{
		return;
	}

	int width = module->GetWidth();
	int height = module->GetHeight();

	for (int i = 0; i < contextNum; ++i)
	{
		WContext* context = new WContext(width, height, module);
		m_contexts.push_back(context);
		m_useable.push(context);
	}

	InitializeCriticalSection(&m_cs);
}

WContextPool::~WContextPool()
{
	for (size_t i = 0; i < m_contexts.size(); ++i)
	{
		delete m_contexts[i];
	}
	DeleteCriticalSection(&m_cs);
}

WContext* WContextPool::GetContext()
{
	WMutex lock(&m_cs);

	WContext* context = m_useable.front();
	m_useable.pop();
	
	return context;
}

void WContextPool::Return(WContext* pContext)
{
	WMutex lock(&m_cs);
	m_useable.push(pContext);
}