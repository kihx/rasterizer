#include "stdafx.h"
#include "OutputMerger.h"

namespace xtozero
{
	const int depthPrecision = 0xFFFFFF;

	COutputMerger::COutputMerger() : m_dpp( 0 ), m_height( 0 ), m_width( 0 ),
		m_pFrameBuffer( nullptr ), m_isFirst( true )
	{
	}


	COutputMerger::~COutputMerger()
	{
	}

	void COutputMerger::CreateDepthBuffer( const int width, const int height )
	{
		if ( m_isFirst )
		{
			m_pDepthBuffer.resize( width * height );
			m_isFirst = false;
		}
	}

	void COutputMerger::ClearDepthBuffer()
	{
		::ZeroMemory( &m_pDepthBuffer[0], m_pDepthBuffer.size() * sizeof(int) );
	}

	void COutputMerger::ClearFrameBuffer( )
	{
		size_t size = m_dpp / 8;

		memset( m_pFrameBuffer, 0, m_width * m_height * size );
	}

	void COutputMerger::SetFrameBuffer( void* pbuffer, int dpp, int width, int height )
	{
		m_dpp = dpp;
		m_width = width;
		m_height = height;
		m_pFrameBuffer = static_cast<unsigned char*>(pbuffer);
	}

	bool COutputMerger::ProcessDepthTest( const int x, const int y, const float depth )
	{
		int index = y * m_width + x;
		assert( m_pDepthBuffer.size( ) >= static_cast<size_t>(index) );

		float InvDepth = (1.0f - depth);
		InvDepth = ( InvDepth < 0 ) ? 0.0f : InvDepth;

		int pixelDepth = static_cast<int>( InvDepth * depthPrecision );
		if ( m_pDepthBuffer[index] < pixelDepth )
		{
			m_pDepthBuffer[index] = pixelDepth;
			return true;
		}

		return false;
	}

	void COutputMerger::Process( const std::vector<COmElementDesc>& omInput )
	{
		assert( m_pFrameBuffer );

		size_t size = m_dpp / 8;

		memset( m_pFrameBuffer, 0, m_width * m_height * size );

		int nPixels = omInput.size();

		for ( int i = 0; i < nPixels; ++i )
		{
			assert( 0 <= omInput.at( i ).m_x && omInput.at( i ).m_x < m_width );
			assert( 0 <= omInput.at( i ).m_y && omInput.at( i ).m_y < m_height );
			const COmElementDesc& input = omInput[i];
			if ( ProcessDepthTest( input.m_x, input.m_y, input.m_z ) )
			{
				unsigned char* pixel = m_pFrameBuffer + ((m_width * input.m_y) + input.m_x) * size;
				pixel[0] = static_cast<unsigned char>( GET_RED( input.m_color ) );
				pixel[1] = static_cast<unsigned char>( GET_GREEN( input.m_color ) );
				pixel[2] = static_cast<unsigned char>( GET_BULE( input.m_color ) );
			}
		}
	}

	void COutputMerger::ProcessParallel( const std::vector<COmElementDesc>& omInput )
	{
		Lock<SpinLock> lock( m_lockObject );

		assert( m_pFrameBuffer );

		size_t size = m_dpp / 8;

		int nPixels = omInput.size( );

		for ( int i = 0; i < nPixels; ++i )
		{
			assert( 0 <= omInput.at( i ).m_x && omInput.at( i ).m_x < m_width );
			assert( 0 <= omInput.at( i ).m_y && omInput.at( i ).m_y < m_height );
			const COmElementDesc& input = omInput[i];
			if ( ProcessDepthTest( input.m_x, input.m_y, input.m_z ) )
			{
				unsigned char* pixel = m_pFrameBuffer + ((m_width * input.m_y) + input.m_x) * size;
				pixel[0] = static_cast<unsigned char>(GET_RED( input.m_color ));
				pixel[1] = static_cast<unsigned char>(GET_GREEN( input.m_color ));
				pixel[2] = static_cast<unsigned char>(GET_BULE( input.m_color ));
			}
		}
	}
}