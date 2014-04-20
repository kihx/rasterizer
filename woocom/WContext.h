#pragma once

#include "common.h"

#include <vector>
#include <memory>

class Vector3;
class Matrix4;
class WModule;
class WContext
{
public:
	explicit WContext(int width, int height, WModule* pModule) 
		: m_width(width), m_height(height), m_module(pModule)
	{
		m_fillInfo.resize(height);
	}
	~WContext(){}

	void ResetFillInfo();
	void MakeLineInfo(const Vector3* v1, const Vector3* v2, const unsigned char* color);
	void InsertLineInfo(int lineIndex, int posX, const unsigned char* rgb);
	void InsertLineDepthInfo(int lineIndex, int posX, float depth, const unsigned char* rgb);
	void SortFillInfo();
	void DrawFillInfo();

	void VertexProcess(Matrix4& mat, Vector3& vertex);

private:
	void DrawScanline(int lineIndex, const EdgeInfo& info);

private:
	WModule* m_module;
	std::vector< EdgeInfo >	m_fillInfo;
	int m_width;
	int m_height;
};