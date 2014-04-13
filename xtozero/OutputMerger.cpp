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

	void COutputMerger::CreateDepthBuffer( const int& width, const int& height )
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

	bool COutputMerger::ProcessDepthTest( const int& x, const int& y, const float& depth )
	{
		assert( m_pDepthBuffer.size() >= m_width * m_height );

		if ( m_pDepthBuffer[y * m_width + x] >= depth * depthPrecision )
		{
			m_pDepthBuffer[y * m_width + x] = static_cast<unsigned char>(depth * depthPrecision);
			return true;
		}

		return false;
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
			assert( 0 <= omInput.at( i ).m_x && omInput.at( i ).m_x < m_width );
			assert( 0 <= omInput.at( i ).m_y && omInput.at( i ).m_y < m_height );
			assert( 0.0f <= omInput.at( i ).m_z && omInput.at( i ).m_z <= 1.0f );
			const COmElementDesc& input = omInput.at( i );
			if ( ProcessDepthTest( input.m_x, input.m_y, input.m_z ) )
			{
				unsigned char* pixel = m_pFrameBuffer + ((m_width * input.m_y) + input.m_x) * size;
				pixel[0] = GET_RED( input.m_color );
				pixel[1] = GET_GREEN( input.m_color );
				pixel[2] = GET_BULE( input.m_color );
			}
		}
	}
}