#pragma once

#include "../utility/math3d.h"

#include "common.h"
#include <vector>

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

	void Render();
	void Clear( void* pImage, int width, int height, unsigned int clearColor );
	void PaintPixel( int x, int y, const unsigned char* rgb);
	void ZBufferPaintPixel(int x, int y, float z, const unsigned char* rgb);
	bool DepthTest(int x, int y, float z);

	void ResetFillInfo();
	void InsertLineInfo(int lineIndex, int posX, const unsigned char* rgb);
	void InsertLineDepthInfo(int lineIndex, int posX, float depth, const unsigned char* rgb);
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
	std::vector<float> m_depthBuffer;

	bool m_isSorted;
	std::vector< EdgeInfo >	m_fillInfo;
	int m_scanOffset;
	int m_scanCount;

	Matrix4	m_world;
	Matrix4 m_view;
	Matrix4 m_proj;
	Matrix4 m_wvp;
};