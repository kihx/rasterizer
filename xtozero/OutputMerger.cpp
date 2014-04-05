#include "stdafx.h"
#include "OutputMerger.h"

namespace xtozero
{
	COutputMerger::COutputMerger( ) : m_pDepthBuffer( nullptr ), m_dpp( 0 ), m_height( 0 ), m_width( 0 ),
		m_pFrameBuffer( nullptr )
	{
	}


	COutputMerger::~COutputMerger()
	{
	}

	void COutputMerger::CreateDepthBuffer( int width, int height )
	{
		m_pDepthBuffer = new unsigned char[height * width];
	}

	void COutputMerger::DestroyDepthBuffer()
	{
		delete []m_pDepthBuffer;
	}

	void COutputMerger::SetFrameBuffer( void* pbuffer, int dpp, int width, int height )
	{
		m_dpp = dpp;
		m_width = width;
		m_height = height;
		m_pFrameBuffer = static_cast<unsigned char*>(pbuffer);
	}

	bool COutputMerger::ProcessDepthTest( int x, int y, float depth )
	{
		if ( m_pDepthBuffer )
		{
			if ( m_width <= x && x < 0 )
			{

			}
			else if ( m_height <= x && y < 0 )
			{

			}
			else
			{
				if ( m_pDepthBuffer[ y * m_width + x ] >= depth )
				{
					m_pDepthBuffer[ y * m_width + x ] = static_cast<unsigned char>(depth);
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		return true;
	}

	void COutputMerger::Process( std::vector<COmElementDesc>& omInput )
	{
		if ( m_pFrameBuffer )
		{
			size_t size = m_dpp / 8;

			unsigned int color = PIXEL_COLOR( 0, 0, 0 );

			for ( int i = 0; i < m_height; ++i )
			{
				for ( int j = 0; j < m_width; ++j )
				{
					memcpy_s( m_pFrameBuffer + ((m_width * i) + j) * size,
						size,
						&color,
						size );
				}
			}

			for ( std::vector<COmElementDesc>::iterator& iter = omInput.begin( ); iter != omInput.end( ); ++iter )
			{
				//if ( ProcessDepthTest( iter->m_x, iter->m_y, iter->m_z ) )
				//{
					memcpy_s( m_pFrameBuffer + ((m_width * iter->m_y) + iter->m_x) * size,
						size,
						&iter->m_color,
						size );
				//}
			}
		}
	}
}