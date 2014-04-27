#pragma once

class WModule;
class WIDrawable
{
public:
	virtual void DrawOutline(WModule* pPainter) = 0;
	virtual void DrawSolid(WModule* pPainter) = 0;
	virtual void DrawSolidParallel(WModule* pModule) = 0;
};