#pragma once

#include "common.h"

#include <vector>
#include <memory>

class WModule;
class WPainter
{
public:
	explicit WPainter(int height, const std::shared_ptr< WModule > pModule) 
		: m_height(height), m_module(pModule)
	{
		m_fillInfo.resize(height);
	}
	~WPainter(){}

	void ResetFillInfo();
	void InsertLineInfo(int lineIndex, int posX, const unsigned char* rgb);
	void InsertLineDepthInfo(int lineIndex, int posX, float depth, const unsigned char* rgb);
	void SortFillInfo();
	void DrawFillInfo();

private:
	void DrawScanline(int lineIndex, const EdgeInfo& info);

private:
	const std::shared_ptr< WModule> m_module;
	std::vector< EdgeInfo >	m_fillInfo;
	int m_height;
};