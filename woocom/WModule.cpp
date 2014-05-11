#define WMODULE_API extern "C" __declspec(dllexport)

#include "woocom.h"
#include "WModule.h"
#include "WFileLoader.h"
#include "WMesh.h"
#include "WPolygon.h"
#include "Utility.h"
#include "Thread.h"
#include "WContextPool.h"
#include "ConCommand.h"

#include <assert.h>
#include <memory>
#include <thread>
#include <iostream>

#include <Windows.h>

std::shared_ptr<WModule> g_pPainter;
std::shared_ptr<WIDrawable> g_pDrawObj;

std::shared_ptr<WThreadPool> g_pool;


typedef void(*RenderFunc)(WModule*);
RenderFunc g_RenderFunction = nullptr;
bool g_isGridScene = false;

void SingleThreadRendering(WModule* pModule)
{	
	g_pDrawObj->DrawSolid(pModule);
}

void MultiThreadRendering(WModule* pModule)
{
	g_pDrawObj->DrawSolidParallel(pModule);
}

DECLARE_CONCOMMAND(toggle_rendering)
{
	if (g_RenderFunction == SingleThreadRendering)
	{
		g_RenderFunction = MultiThreadRendering;
		std::cout << "MultiThreading." << std::endl;
	}
	else
	{
		g_RenderFunction = SingleThreadRendering;
		std::cout << "SingleThreading." << std::endl;
	}
}

DECLARE_CONCOMMAND(grid_scene)
{
	g_isGridScene = !g_isGridScene;
}

WModule::WModule() : m_isInit(false), m_depthBuffer(nullptr)
{
	g_RenderFunction = SingleThreadRendering;
}

WModule::~WModule()
{
	if (m_depthBuffer != nullptr)
	{
		delete[] m_depthBuffer;
	}
}

void WModule::Init(void* buffer, int width, int height, int bpp)
{
	m_isInit = true;
	m_buffer = buffer;
	m_screenWidth = width;
	m_screenHeight = height;
	m_colorDepth = bpp;

	m_scanOffset = height;
	m_scanCount = 0;

	int pixelNum = width * height;
	m_depthBuffer = new float[pixelNum];
	for (int i = 0; i < pixelNum; ++i)
	{
		m_depthBuffer[i] = 1.0f;
	}

	m_fillInfo.resize(height);

	const unsigned threadNum = std::thread::hardware_concurrency();
	g_pool = std::make_shared<WThreadPool>(threadNum);
	m_contextPool = new WContextPool(threadNum, this);
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

	int pixelNum = width * height;
	for (int i = 0; i < pixelNum; ++i)
	{
		m_depthBuffer[i] = 1.0f;
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

void WModule::ZBufferPaintPixel(int x, int y, float z, const unsigned char* rgb)
{
	if (DepthTest(x, y, z))
	{
		PaintPixel(x, y, rgb);
	}
}

bool WModule::DepthTest(int x, int y, float z)
{
	// 클리핑
	if (x < 0 || x > m_screenWidth)
	{
		return false;
	}

	if (y < 0 || y > m_screenHeight)
	{
		return false;
	}
	
	// depth test
	int depthIndex = m_screenWidth * y + x;
	if (m_depthBuffer[depthIndex] < z)
	{
		return false;
	}
	else
	{
		m_depthBuffer[depthIndex] = z;
		return true;
	}
}

WContext* WModule::GetContext()
{
	if (m_contextPool == nullptr)
	{
		return nullptr;
	}

	return m_contextPool->GetContext();
}

void WModule::ReturnContext(WContext* pContext)
{
	m_contextPool->Return(pContext);
}

void WModule::ResetFillInfo()
{
	// 라인정보 비우기
	size_t num = m_fillInfo.size();
	for (size_t i = 0; i < num; ++i)
	{
		m_fillInfo[i].m_edgeData.clear();
	}

	m_scanOffset = m_screenHeight;
	m_scanCount = 0;
}

void WModule::InsertLineInfo(size_t lineIndex, int posX, const unsigned char* rgb)
{
	assert((lineIndex >= 0 && lineIndex < (size_t)m_screenHeight) && "fillInfo Index out of range");

	m_fillInfo[lineIndex].Insert(posX, rgb);

	// 엣지 정보가 들어있는 공간을 저장
	// offset 부터 count 갯수만큼 그리도록
	if (m_scanOffset > lineIndex)
	{
		m_scanOffset = lineIndex;
	}
	else if (m_scanCount < lineIndex)
	{
		m_scanCount = lineIndex;
	}

	m_isSorted = false;
}

void WModule::InsertLineDepthInfo(size_t lineIndex, int posX, float depth, const unsigned char* rgb)
{
	assert((lineIndex >= 0 && lineIndex < (size_t)m_screenHeight) && "fillInfo Index out of range");

	m_fillInfo[lineIndex].Insert(posX, depth, rgb);

	// 엣지 정보가 들어있는 공간을 저장
	// offset 부터 count 갯수만큼 그리도록
	if (m_scanOffset > lineIndex)
	{
		m_scanOffset = lineIndex;
	}
	else if (m_scanCount < lineIndex)
	{
		m_scanCount = lineIndex;
	}

	m_isSorted = false;
}

void WModule::SortFillInfo()
{
	for (size_t i = m_scanOffset; i < m_scanCount; ++i)
	{
		m_fillInfo[i].Sort();
	}

	m_isSorted = true;
}

void WModule::DrawFillInfo()
{
	for (size_t i = m_scanOffset; i < m_scanCount; ++i)
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

void WModule::RenderScene()
{
	if (g_pDrawObj == nullptr)
	{
		return;
	}

	if (g_isGridScene)
	{
		float scale = 0.1f;
		Matrix4 matScale;
		matScale.Scaling(scale, scale, scale);

		Matrix4 matWorld = GetWorld();
		Matrix4 matTranslate;

		float cellWidth = 0.7f;
		float cellHeight = 0.7f;
		float cellNum = 5.0f;
		float totalWidth = cellWidth * cellNum;
		float totalHeight = cellHeight * cellNum;

		float halfWidth = totalWidth / 2.0f;
		float halfHeight = totalHeight / 2.0f;

		for (float cellX = -halfWidth; cellX < halfWidth; cellX += cellWidth)
		{
			for (float cellZ = -halfHeight; cellZ < halfHeight; cellZ += cellHeight)
			{
				// translate
				matTranslate.Translate(cellX, 0, cellZ);

				matTranslate = matScale * matWorld * matTranslate;
				SetTransform(0, matTranslate);
				
				g_RenderFunction(this);
			}
		}
	}
	else
	{
		g_RenderFunction(this);
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
	UNREFERENCED_PARAMETER(hInst);
	UNREFERENCED_PARAMETER(fImpLoad);

	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		ConCommandExecuter::Destory();
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
	
	g_pPainter->RenderScene();
	
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

void WExecuteCommand(const char* command)
{
	ConCommandExecuter::GetInstance()->Execute(command);
}