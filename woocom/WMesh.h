#pragma once

#include "WTrDatai.h"

#include <memory>

class WModule;
class WMesh
{
public:
	WMesh(WTriData* data) 
		: m_data(data)
	{
	}
	~WMesh()
	{
		delete m_data;
	}

	void DrawOutline( WModule* pPainter);
	void DrawSolid( WModule* pPainter);
private:
	void DrawLine(WModule* pPainter, VERTEX* v1, VERTEX* v2, unsigned char* rgb);

	WTriData* m_data;
};