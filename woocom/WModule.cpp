#define WMODULE_API extern "C" __declspec(dllexport)

#include <Windows.h>
#include "WModule.h"
#include "WTrDatai.h"

#include <fstream>

#define MAX_LEN 1024

class WModule
{
public:
	WModule(int bpp);
	~WModule();

	void Render( void* buffer, int width, int height, int bpp);
	void Clear( void* pImage, int width, int height, unsigned long clearColor );
private:
	int m_colorDepth;
};

WModule* g_pPainter = nullptr;
WTriData* g_pTriangle = nullptr;

WModule::WModule(int bpp) : m_colorDepth(bpp)
{
}

WModule::~WModule()
{
}

void WModule::Render(void* buffer, int width, int height, int bpp)
{
	int bufferSize = width * height * bpp / 8;
	::memset( buffer, 128, bufferSize);
}

void WModule::Clear(void* pImage, int width, int height, unsigned long clearColor)
{
	char* buffer = (char*)pImage;
	int loopCount = (width * height * m_colorDepth / 8) - ( m_colorDepth / 8 );
	for( int i = 0; i < loopCount; i = i + m_colorDepth )
	{
		buffer[i] = clearColor >> 16 & 0xff;
		buffer[i+1] = clearColor >> 8 & 0xff;
		buffer[i+2] = clearColor & 0xff;
	}
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, PVOID fImpLoad)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		if ( g_pPainter )
		{
			delete g_pPainter;
			g_pPainter = nullptr;
		}
		break;
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}

// dll export function
void WRender(void* buffer, int width, int height, int colorDepth)
{
	if( g_pPainter == nullptr )
	{
		g_pPainter = new WModule(colorDepth);
	}

	g_pPainter->Render( buffer, width, height, colorDepth );
}

void WClear( void* pImage, int width, int height, unsigned long clearColor )
{
	if( g_pPainter )
	{
		g_pPainter->Clear( pImage, width, height, clearColor );
	}
}

void WLoadMesh( const char* file )
{
	std::fstream stream( file );
	
	char buffer[MAX_LEN] = {0};
	char line[MAX_LEN] = {0};

	int vertexNum = 0;
	int faceNum = 0;

	WTriData* triangle = new WTriData();

	while( stream.good() )
	{
		stream >> buffer;

		if( strstr( buffer, "#$"))
		{
			if( _stricmp( &buffer[2], "Vertices") == 0)
			{
				stream >> vertexNum;
				triangle->SetVertexNum( vertexNum );
			}
			else if( _stricmp( &buffer[2], "Faces") == 0)
			{
				stream >> faceNum;
				triangle->SetFaceNum( faceNum );
			}
		}
		else if( _stricmp( buffer, "Vertex") == 0)
		{
			int vertexID = 0;
			VERTEX* vertex = new VERTEX();
			stream >> vertexID >> vertex->m_pos[0] >> vertex->m_pos[1] >> vertex->m_pos[2];
			triangle->PushVertex( vertex );
		}
		else if( _stricmp( buffer, "Face") == 0)
		{
			int faceID = 0;
			int indexNum = 0;
			int r,g,b;
			WFace* face = new WFace();
			stream >> faceID >> r >> g >> b;
			face->m_rgb[0] = r;
			face->m_rgb[1] = g;
			face->m_rgb[2] = b;
			
			stream >> indexNum;
			for(int i=0; i< indexNum; ++i)
			{
				int vertexID = 0;
				stream >> vertexID;
				face->m_index.push_back( vertexID );
			}
			triangle->PushFace( face );
		}
	}
	
	if( g_pTriangle )
	{
		delete g_pTriangle;
	}
	g_pTriangle = triangle;
}