#pragma once

#include "../utility/math3d.h"

#include <vector>
#include <map>
#include <algorithm>

struct PixelInfo
{
	PixelInfo(int x, const unsigned char* rgb) :m_x(x), m_z(0.0f)
	{
		m_rgb[0] = rgb[0];
		m_rgb[1] = rgb[1];
		m_rgb[2] = rgb[2];
	}
	PixelInfo(int x, float z, const unsigned char* rgb) : m_x(x), m_z(z)
	{
		m_rgb[0] = rgb[0];
		m_rgb[1] = rgb[1];
		m_rgb[2] = rgb[2];
	}

	bool operator < (const PixelInfo& rhs)
	{
		return m_x < rhs.m_x;
	}

	int m_x;
	float m_z;
	unsigned char m_rgb[3];
};

struct EdgeInfo
{
	EdgeInfo(){}
	EdgeInfo(int x, const unsigned char* rgb)
	{
		m_edgeData.emplace_back(x, rgb);
	}
	EdgeInfo(int x, float z, const unsigned char* rgb)
	{
		m_edgeData.emplace_back(x, z, rgb);
	}

	void Insert(int x, const unsigned char* rgb)
	{
		m_edgeData.emplace_back(x, rgb);
	}
	void Insert(int x, float z, const unsigned char* rgb)
	{
		m_edgeData.emplace_back(x, z, rgb);
	}
	void Sort()
	{
		std::sort(m_edgeData.begin(), m_edgeData.end());
	}

	std::vector<PixelInfo> m_edgeData;
};

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

	// ���ؽ� Transform
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

	Matrix4	m_world;
	Matrix4 m_view;
	Matrix4 m_proj;
	Matrix4 m_wvp;
};