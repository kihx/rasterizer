#pragma once

#include "WIDrawable.h"
#include "WPolyData.h"

class Vector3;
class WPolygon : public WIDrawable
{
public:
	WPolygon(WPolyData* poly) : m_data(poly)
	{}
	virtual ~WPolygon()
	{
		delete m_data;
	}

	virtual void DrawOutline(WModule* pPainter);
	virtual void DrawSolid(WModule* pPainter);
	virtual void DrawSolidParallel(WModule* pModule);
private:
	void DrawLine(WModule* pPainter, const Vector3* v1, const Vector3* v2, const unsigned char* color);
	void InsertLineInfo(WModule* pPainter, const Vector3* v1, const Vector3* v2, const unsigned char* color);

	WPolyData*	m_data;
};