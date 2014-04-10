#define WMODULE_API extern "C" __declspec(dllexport)

#include "woocom.h"
#include "WModule.h"
#include "WFileLoader.h"
#include "WMesh.h"
#include "WPolygon.h"
#include "Utility.h"

#include <assert.h>
#include <memory>

#include <Windows.h>

std::shared_ptr<WModule> g_pPainter;
std::shared_ptr<WIDrawable> g_pDrawObj;

WModule::WModule() : m_isInit(false)
{
}

WModule::~WModule()
{
}

void WModule::Init(void* buffer, int width, int height, int bpp)
{
	m_isInit = true;
	m_buffer = buffer;
	m_screenWidth = width;
	m_screenHeight = height;
	m_colorDepth = bpp;

	m_depthBuffer.resize(width * height, 1.0f);
}

bool WModule::IsInitialized()
{
	return m_isInit;
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
	for( int i = 0; i < loopCount; i = i + m_colorDepth / 8 )
	{
		buffer[i] = clearColor >> 16 & 0xff;
		buffer[i+1] = clearColor >> 8 & 0xff;
		buffer[i+2] = clearColor & 0xff;
	}
	m_depthBuffer.clear();
	m_depthBuffer.resize(width * height, 1.0f);
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

void WModule::ZBufferPaintPixel(int x, int y, float z, const unsigned char* rgb)
{
	if (DepthTest(x, y, z))
	{
		PaintPixel(x, y, rgb);
	}
}

bool WModule::DepthTest(int x, int y, float z)
{
	// depth test
	int depthIndex = m_screenWidth * y + x;
	if (m_depthBuffer[depthIndex] <= z)
	{
		return false;
	}
	else
	{
		m_depthBuffer[depthIndex] = z;
		return true;
	}
}

void WModule::ResetFillInfo()
{
	// 라인정보 비우기
	size_t num = m_fillInfo.size();
	for (size_t i = 0; i < num; ++i)
	{
		m_fillInfo[i].m_edgeData.clear();
	}
}

void WModule::InsertLineInfo(int lineIndex, int posX, const unsigned char* rgb)
{
	assert((lineIndex >= 0 && lineIndex < m_screenHeight) && "fillInfo Index out of range");

	m_fillInfo[lineIndex].Insert(posX, rgb);

	m_isSorted = false;
}

void WModule::InsertLineDepthInfo(int lineIndex, int posX, float depth, const unsigned char* rgb)
{
	assert((lineIndex >= 0 && lineIndex < m_screenHeight) && "fillInfo Index out of range");

	m_fillInfo[lineIndex].Insert(posX, depth, rgb);
	m_isSorted = false;
}

void WModule::SortFillInfo()
{
	size_t num = m_fillInfo.size();
	for (size_t i = 0; i < num; ++i)
	{
		m_fillInfo[i].Sort();
	}

	m_isSorted = true;
}

void WModule::DrawFillInfo()
{
	size_t num = m_fillInfo.size();
	for (size_t i = 0; i < num; ++i)
	{
		DrawScanline(i, m_fillInfo[i]);
	}
}

void WModule::DrawScanline(int lineIndex, const EdgeInfo& info)
{
	size_t edgeInfoCount = info.m_edgeData.size();

	// 엣지 정보가 홀수이면 assert 경고
	assert(!(edgeInfoCount & 1) && "Invalid edgeInfoCount.");

	for (size_t i = 0; i + 1 < edgeInfoCount; i += 2)
	{
		const PixelInfo& first = info.m_edgeData[i];
		const PixelInfo& second = info.m_edgeData[i + 1];

		if (first.m_x == second.m_x)
		{
			ZBufferPaintPixel(first.m_x, lineIndex, first.m_z, first.m_rgb);
			return;
		}

		float dx = (float)(second.m_x - first.m_x);
		float dz = (float)(second.m_z - first.m_z);
		for (int offsetX = first.m_x; offsetX <= second.m_x; ++offsetX)
		{
			float z = first.m_z + dz * ((offsetX - first.m_x) / dx);
			ZBufferPaintPixel(offsetX, lineIndex, z, first.m_rgb);
		}
	}
}

void WModule::VertexProcess( Matrix4& mat, Vector3& vertex)
{
	vertex.Transform(mat);

	float fHalfWidth = m_screenWidth * 0.5f;
	float fHalfHeight = m_screenHeight * 0.5f;
	
	vertex.X = vertex.X * fHalfWidth + fHalfWidth;
	vertex.Y = -vertex.Y * fHalfHeight + fHalfHeight;
}

void WModule::SetTransform(int type, const Matrix4& transform)
{
	switch (type)
	{
	case 0:
		m_world = transform;
		break;
	case 1:
		m_view = transform;
		break;
	case 2:
		m_proj = transform;
		break;
	default:
		assert(!"Invalid transform type");
		break;
	}
}

const Matrix4& WModule::GetWorld() const
{
	return m_world;
}

const Matrix4& WModule::GetView() const
{
	return m_view;
}

const Matrix4& WModule::GetProj() const
{
	return m_proj;
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
		g_pPainter = std::shared_ptr<WModule>(new WModule());
		g_pPainter->Init(buffer, width, height, colorDepth);
	}

	if (!g_pPainter->IsInitialized())
	{
		g_pPainter->Init(buffer, width, height, colorDepth);
	}

	//g_pPainter->Render();
	g_pPainter->Clear(buffer, width, height, 0x000000);
	
	if (g_pDrawObj)
	{
		//g_pDrawObj->DrawOutline( g_pPainter.get() );
		g_pDrawObj->DrawSolid(g_pPainter.get());
	}
	
}

void WClear( void* pImage, int width, int height, unsigned long clearColor )
{
	if( g_pPainter )
	{
		g_pPainter->Clear( pImage, width, height, clearColor );
	}
}

void WTransform(int transformType, const float* matrix4x4)
{
	if (g_pPainter == nullptr)
	{
		g_pPainter = std::shared_ptr<WModule>(new WModule());
	}

	if (g_pPainter)
	{
		g_pPainter->SetTransform(transformType, Utility::Float2Matrix4(matrix4x4));
	}
}

void WLoadMesh(const char* fileName)
{
	if (strstr(fileName, ".msh"))
	{
		WTriData* triangle = FileLoader::LoadMesh(fileName);
		g_pDrawObj = std::shared_ptr<WIDrawable>(new WMesh(triangle));
	}
	else if (strstr(fileName, ".ply"))
	{
		WPolyData* polygon = FileLoader::LoadPoly(fileName);
		g_pDrawObj = std::shared_ptr<WIDrawable>(new WPolygon(polygon));
	}
}