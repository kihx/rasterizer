#pragma once

#include "../utility/math3d.h"

#include <vector>
#include <map>
#include <algorithm>

struct PixelInfo
{
	PixelInfo(int x, const unsigned char* rgb) :m_x(x)
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
	unsigned char m_rgb[3];
};

struct EdgeInfo
{
	EdgeInfo(){}
	EdgeInfo(int x, const unsigned char* rgb)
	{
		m_edgeData.emplace_back(x, rgb);
	}

	void Insert(int x, const unsigned char* rgb)
	{
		m_edgeData.emplace_back(x, rgb);
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
	WModule(void* buffer, int width, int height, int bpp);
	~WModule();

	void Render();
	void Clear( void* pImage, int width, int height, unsigned int clearColor );
	void PaintPixel( int x, int y, const unsigned char* rgb);
	bool DepthTest(int x, int y, float z);

	void ResetFillInfo();
	void InsertLineInfo(int lineIndex, int posX, const unsigned char* rgb);
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