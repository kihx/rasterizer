#define WMODULE_API extern "C" __declspec(dllexport)

#include <Windows.h>
#include "WModule.h"
#include "WTrDatai.h"

#include <fstream>

#define MAX_LEN 1024

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

WTriData* LoadMesh( const char* file )
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

	return triangle;
}