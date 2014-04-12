#include "stdafx.h"
#include "OutputMerger.h"

namespace xtozero
{
	const int depthPrecision = 255;

	COutputMerger::COutputMerger( ) : m_dpp( 0 ), m_height( 0 ), m_width( 0 ),
		m_pFrameBuffer( nullptr )
	{
	}


	COutputMerger::~COutputMerger()
	{
	}

	void COutputMerger::CreateDepthBuffer( int width, int height )
	{
		if ( m_pDepthBuffer.size( ) < width * height )
		{
			m_pDepthBuffer.resize( width * height );
		}
	}

	void COutputMerger::ClearDepthBuffer()
	{
		for ( int i = 0; i < m_width * m_height; ++i )
		{
			m_pDepthBuffer[i] = 255;
		}
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
		assert( m_pDepthBuffer.size() >= m_width * m_height );

		if ( m_pDepthBuffer[y * m_width + x] >= depth * depthPrecision )
		{
			m_pDepthBuffer[y * m_width + x] = static_cast<unsigned char>(depth * depthPrecision);
			return true;
		}
		else
		{
			return false;
		}
		return true;
	}

	void COutputMerger::Process( const std::vector<COmElementDesc>& omInput )
	{
		assert( m_pFrameBuffer );

		size_t size = m_dpp / 8;

		unsigned int color = PIXEL_COLOR( 0, 0, 0 );

		memset( m_pFrameBuffer, 0, m_width * m_height * size );

		int nPixels = omInput.size();

		for ( int i = 0; i < nPixels; ++i )
		{
			assert( 0 <= iter->m_x && iter->m_x < m_width );
			assert( 0 <= iter->m_y && iter->m_y < m_height );
			assert( 0.0f <= iter->m_z && iter->m_z <= 1.0f );
			const COmElementDesc& input = omInput.at( i );
			if ( ProcessDepthTest( input.m_x, input.m_y, input.m_z ) )
			{
				memcpy_s( m_pFrameBuffer + ((m_width * input.m_y) + input.m_x) * size,
					size,
					&input.m_color,
					size );
			}
		}
	}
}