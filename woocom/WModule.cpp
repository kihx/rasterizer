#define WMODULE_API extern "C" __declspec(dllexport)

#include "WModule.h"
#include "WTrDatai.h"
#include "WMesh.h"

#include <memory>
#include <fstream>

#include <Windows.h>

#define MAX_LEN 1024


std::shared_ptr<WModule> g_pPainter;
std::shared_ptr<WMesh> g_pMesh;

WModule::WModule(void* buffer, int width, int height, int bpp) 
	: m_buffer(buffer), m_screenWidth(width), m_screenHeight(height), m_colorDepth(bpp)
{
}

WModule::~WModule()
{
}

void WModule::Render()
{
	int bufferSize = m_screenWidth * m_screenHeight * m_colorDepth / 8;
	::memset( m_buffer, 128, bufferSize);
}

void WModule::Clear(void* pImage, int width, int height, unsigned int clearColor)
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

void WModule::PaintPixel(int x, int y, unsigned char* rgb)
{
	if( m_buffer == nullptr )
	{
		return;
	}

	// Å¬¸®ÇÎ
	if( x < 0 || x > m_screenWidth )
	{
		return;
	}

	if( y < 0 || y > m_screenHeight )
	{
		return;
	}

	char* buffer = (char*)m_buffer;

	int index =	(m_screenWidth * y) * ( m_colorDepth / 8 ) + ( x * m_colorDepth / 8);
	buffer[index] = rgb[0];
	buffer[index + 1] = rgb[1];
	buffer[index + 2] = rgb[2];
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
		g_pPainter = std::shared_ptr<WModule>(new WModule(buffer, width, height, colorDepth));
	}

	g_pPainter->Render();
	
	if( g_pMesh )
	{
		g_pMesh->DrawOutline( g_pPainter.get() );
	}
	
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
			if( strncmp( &buffer[2], "Vertices", 8) == 0)
			{
				stream >> vertexNum;
				triangle->SetVertexNum( vertexNum );
			}
			else if( strncmp( &buffer[2], "Faces", 5) == 0)
			{
				stream >> faceNum;
				triangle->SetFaceNum( faceNum );
			}
		}
		else if( strncmp( buffer, "Vertex", 6) == 0)
		{
			int vertexID = 0;
			VERTEX* vertex = new VERTEX();
			stream >> vertexID >> vertex->m_pos[0] >> vertex->m_pos[1] >> vertex->m_pos[2];
			triangle->PushVertex( vertex );
		}
		else if( strncmp( buffer, "Face", 4) == 0)
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
				face->m_index.push_back( vertexID -1 );
			}
			triangle->PushFace( face );
		}
	}
	
	g_pMesh = std::shared_ptr<WMesh>( new WMesh( triangle ));
}