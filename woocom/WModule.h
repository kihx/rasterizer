#pragma once

#ifdef WMODULE_API

#else
#define WMODULE_API extern "C" __declspec(dllimport)
#endif

WMODULE_API void WLoadMesh( const char* filename );
WMODULE_API void WRender( void* buffer, int width, int height, int bpp );
WMODULE_API void WClear( void* pImage, int width, int height, unsigned long clearColor );

class WMesh;
class WModule
{
public:
	WModule(void* buffer, int width, int height, int bpp);
	~WModule();

	void Render();
	void Render(WMesh* pMesh);
	void Clear( void* pImage, int width, int height, unsigned long clearColor );
private:
	void* m_buffer;
	int m_screenWidth;
	int m_screenHeight;
	int m_colorDepth;
};