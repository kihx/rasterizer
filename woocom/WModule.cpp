#define WMODULE_API extern "C" __declspec(dllexport)

#include <Windows.h>
#include "WModule.h"

class WModule
{
public:
	WModule();
	~WModule();

	bool Initialize( void* buffer, int width, int height, int colorDepth);
	void Clear( uchar r, uchar g, uchar b);

private:
	int m_width;
	int m_height;
	int m_colorDepth;

	void* m_buffer;
	int m_bufferSize;
	bool m_isInitalized;
};

WModule* g_pPainter = NULL;

WModule::WModule(): m_width(0), m_height(0), m_colorDepth(0), m_buffer(NULL), m_bufferSize(0), m_isInitalized(false)
{
}

WModule::~WModule()
{
}

bool WModule::Initialize(void* buffer, int width, int height, int colorDepth)
{
	m_buffer = buffer;
	m_width = width;
	m_height = height;
	m_colorDepth = colorDepth;

	m_bufferSize = width * height * colorDepth;
	m_isInitalized = m_bufferSize > 0;

	return m_isInitalized;
}

void WModule::Clear(uchar r, uchar g, uchar b)
{
	if( m_isInitalized )
	{
		uchar* buffer = (uchar*)m_buffer;
		int loopCount = m_bufferSize - m_colorDepth;
		for( int i = 0; i < loopCount; i = i + m_colorDepth )
		{
			buffer[i] = r;
			buffer[i+1] = g;
			buffer[i+2] = b;
		}			
	}
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, PVOID fImpLoad)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_pPainter = new WModule();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		delete g_pPainter;
		g_pPainter = NULL;
		break;
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}

// dll export function
bool Initialize(void* buffer, int width, int height, int colorDepth)
{
	if( g_pPainter )
	{
		return g_pPainter->Initialize( buffer, width, height, colorDepth );
	}
	return false;
}

void Clear(uchar r, uchar g, uchar b)
{
	if( g_pPainter )
	{
		g_pPainter->Clear( r, g, b);
	}
}