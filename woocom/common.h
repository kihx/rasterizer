#pragma once

#include <vector>
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