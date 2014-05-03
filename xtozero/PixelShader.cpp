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

		m_psOutput.resize( size );

		for ( int i = 0; i < size; ++i )
		{
			const CPsElementDesc& ominput = psInput[i];
			m_psOutput[i].m_x = ominput.m_x;
			m_psOutput[i].m_y = ominput.m_y;
			m_psOutput[i].m_z = ominput.m_z;
			m_psOutput[i].m_color = ominput.m_color;
		}

		return m_psOutput;
	}
}