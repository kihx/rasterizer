#include "stdafx.h"
#include "PixelShader.h"

namespace xtozero
{
	CPixelShader::CPixelShader()
	{
	}


	CPixelShader::~CPixelShader()
	{
	}

	const std::vector<COmElementDesc>& CPixelShader::Process( const std::vector<CPsElementDesc>& psInput )
	{
		m_psOutput.clear();
		int size = psInput.size( );

		m_psOutput.reserve( size );

		for ( int i = 0; i < size; ++i )
		{
			const CPsElementDesc& ominput = psInput.at( i );
			m_psOutput.emplace_back( ominput.m_x, ominput.m_y, ominput.m_z, ominput.m_color );
		}

		return m_psOutput;
	}
}