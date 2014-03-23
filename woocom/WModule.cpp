#define WMODULE_API extern "C" __declspec(dllexport)

#include "woocom.h"
#include "WModule.h"
#include "WTrDatai.h"
#include "WMesh.h"

#include <memory>
#include <fstream>

#include <Windows.h>

std::shared_ptr<WModule> g_pPainter;
std::shared_ptr<WMesh> g_pMesh;

WModule::WModule(void* buffer, int width, int height, int bpp) 
	: m_buffer(buffer), m_screenWidth(width), m_screenHeight(height), m_colorDepth(bpp),
	m_isSorted(false)
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

void WModule::PaintPixel(int x, int y, const unsigned char* rgb)
{
	if( m_buffer == nullptr )
	{
		return;
	}

	// 클리핑
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

void WModule::ResetFillInfo()
{
	// 라인정보 비우기
	m_fillInfo.clear();
}

void WModule::InsertLineInfo(int lineIndex, int posX, const unsigned char* rgb)
{
	const auto& itr = m_fillInfo.find(lineIndex);
	if ( m_fillInfo.find(lineIndex) == m_fillInfo.end() )
	{
		m_fillInfo.emplace(lineIndex, EdgeInfo(posX, rgb));
	}
	else
	{
		m_fillInfo[lineIndex].Insert(posX, rgb);
	}

	m_isSorted = false;
}

void WModule::SortFillInfo()
{
	auto& itr = m_fillInfo.begin();
	for (; itr != m_fillInfo.end(); ++itr)
	{
		itr->second.Sort();
	}

	m_isSorted = true;
}

void WModule::DrawFillInfo()
{
	auto& itr = m_fillInfo.begin();
	for (; itr != m_fillInfo.end(); ++itr)
	{
		DrawScanline(itr->first, itr->second);
	}
}

void WModule::DrawScanline(int lineIndex, const EdgeInfo& info)
{
	// 엣지 정보가 짝수 이어야 하는데 홀수로 들어오는 부분이 있음
	// 버그 수정이 필요
	size_t edgeInfoCount = info.m_edgeData.size() - 1;
	for (size_t i = 0; i < edgeInfoCount; i += 2)
	{
		const PixelInfo& first = info.m_edgeData[i];
		const PixelInfo& second = info.m_edgeData[i + 1];

		if (first.m_x == second.m_x)
		{
			PaintPixel(first.m_x, lineIndex, first.m_rgb);
			return;
		}

		for (int offsetX = first.m_x; offsetX <= second.m_x; ++offsetX)
		{
			PaintPixel(offsetX, lineIndex, first.m_rgb);
		}
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
		//g_pMesh->DrawOutline( g_pPainter.get() );
		g_pMesh->DrawSolid(g_pPainter.get());
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
	
	const int MAX_LEN = 1024;
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