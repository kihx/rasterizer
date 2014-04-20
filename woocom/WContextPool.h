#pragma once

#include <vector>
#include <queue>
#include <windows.h>

class WModule;
class WContext;
class WContextPool
{
public:
	WContextPool(int contextNum, WModule* module);
	~WContextPool();

	WContext* GetContext();
	void Return(WContext* pContext);
	
private:
	CRITICAL_SECTION m_cs;
	std::vector<WContext*> m_contexts;
	std::queue<WContext*> m_useable;
};