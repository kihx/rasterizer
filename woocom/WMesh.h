#pragma once

#include "WTrDatai.h"

#include <memory>

class WModule;
class WMesh
{
public:
	WMesh(WTriData* data, const std::unique_ptr<WModule>& painter) 
		: m_data(data), m_Painter(painter){}
	~WMesh()
	{
		delete m_data;
	}

	void DrawOutline();
	void DrawSolid();
private:
	void DrawLine(VERTEX* v1, VERTEX* v2, unsigned char* rgb);

	WTriData* m_data;
	const std::unique_ptr<WModule>& m_Painter;
};