#pragma once

#include "WTrDatai.h"
#include "WIDrawable.h"

#include <memory>

class WMesh : public WIDrawable
{
public:
	WMesh(WTriData* data) 
		: m_data(data)
	{
	}
	virtual ~WMesh()
	{
		delete m_data;
	}

	virtual void DrawOutline( WModule* pPainter);
	virtual void DrawSolid( WModule* pPainter);
	virtual void DrawSolidParallel(WModule* pModule);
private:
	void DrawLine(WModule* pPainter, const VERTEX* v1, const VERTEX* v2, const unsigned char* rgb);
	void InsertLineInfo(WModule* pPainter, const VERTEX* v1, const VERTEX* v2, const unsigned char* rgb);

	WTriData* m_data;
};