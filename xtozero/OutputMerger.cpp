#include "stdafx.h"
#include "OutputMerger.h"

namespace xtozero
{
	COutputMerger::COutputMerger() : m_ppDepthBuffer( nullptr ), m_height( 0 ), m_width( 0 ),
		m_pFrameBuffer( nullptr )
	{
	}


	COutputMerger::~COutputMerger()
	{
	}

	void COutputMerger::CreateDepthBuffer( int width, int height )
	{
		m_width = width;
		m_height = height;

		m_ppDepthBuffer = new unsigned char*[height];
		for ( int i = 0; i < height; ++i )
		{
			m_ppDepthBuffer[i] = new unsigned char[width];
		}
	}

	void COutputMerger::DestroyDepthBuffer()
	{
		for ( int i = 0; i < m_height; ++i )
		{
			delete m_ppDepthBuffer[i];
		}
		delete m_ppDepthBuffer;
	}

	void COutputMerger::SetFrameBuffer( void* pbuffer, int dpp )
	{
		m_dpp = dpp;
		m_pFrameBuffer = static_cast<BYTE*>(pbuffer);
	}

	bool COutputMerger::ProcessDepthTest( int x, int y, float depth )
	{
		if ( m_ppDepthBuffer )
		{
			if ( m_width <= x && x < 0 )
			{

			}
			else if ( m_height <= x && y < 0 )
			{

			}
			else
			{
				if ( m_ppDepthBuffer[x][y] >= depth )
				{
					m_ppDepthBuffer[x][y] = depth;
					return true;
				}
				else
				{
					return false;
				}
			}
		}
	}

	void COutputMerger::Process( std::vector<COmElementDesc>& OmInput )
	{
		if ( m_pFrameBuffer )
		{
			size_t size = m_dpp / 8;

			for ( std::vector<COmElementDesc>::iterator iter = OmInput.begin(); iter != OmInput.end(); ++iter )
			{
				if ( ProcessDepthTest( iter->m_x, iter->m_y, iter->m_z ) )
				{
					memcpy_s( m_pFrameBuffer + ((m_width * iter->m_y) + iter->m_x) * size,
						size,
						&iter->m_color,
						size );
				}
			}
		}
	}
}