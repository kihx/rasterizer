#pragma once

#include "Math.h"

#include "common.h"
#include <mutex>
#include <vector>

class WContext;
class WContextPool;
class WModule
{
	enum class TransformType : int
	{
		World = 0,
		View,
		Projection,
	};

public:
	WModule();
	~WModule();

	void Init(void* buffer, int width, int height, int bpp);
	bool IsInitialized();
	int GetHeight(){ return m_screenHeight; }
	int GetWidth(){ return m_screenWidth; }

	void Render();
	void RenderScene();
	void Clear( void* pImage, int width, int height, unsigned int clearColor );
	void PaintPixel( int x, int y, const unsigned char* rgb);
	void ZBufferPaintPixel(int x, int y, float z, const unsigned char* rgb);
	bool DepthTest(int x, int y, float z);
	WContext* GetContext();
	void ReturnContext(WContext* pContext);

	void ResetFillInfo();
	void InsertLineInfo(size_t lineIndex, int posX, const unsigned char* rgb);
	void InsertLineDepthInfo(size_t lineIndex, int posX, float depth, const unsigned char* rgb);
	void SortFillInfo();
	void DrawFillInfo();

	// ¹öÅØ½º Transform
	void VertexProcess(Matrix4& mat, Vector3& vertex);

	void SetTransform(int type, const Matrix4& transform);
	const Matrix4& GetWorld() const;
	const Matrix4& GetView() const;
	const Matrix4& GetProj() const;

private:
	void DrawScanline(int lineIndex, const EdgeInfo& info );

private:
	bool m_isInit;
	void* m_buffer;
	int m_screenWidth;
	int m_screenHeight;
	int m_colorDepth;
	float* m_depthBuffer;

	bool m_isSorted;
	std::vector< EdgeInfo >	m_fillInfo;
	size_t m_scanOffset;
	size_t m_scanCount;

	std::mutex m_mutex;

	Matrix4	m_world;
	Matrix4 m_view;
	Matrix4 m_proj;
	Matrix4 m_wvp;

	WContextPool* m_contextPool;
};