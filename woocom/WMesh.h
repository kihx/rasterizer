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
	void DrawLine(WModule* pPainter, const VERTEX* v1, const VERTEX* v2, const unsigned char* rgb);
	void InsertLineInfo(WModule* pPainter, const VERTEX* v1, const VERTEX* v2, const unsigned char* rgb);

	WTriData* m_data;
};