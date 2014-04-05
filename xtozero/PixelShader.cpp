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

	std::vector<COmElementDesc> CPixelShader::Process( std::vector<CPsElementDesc>& psInput )
	{
		std::vector<COmElementDesc> psOutput;
		psOutput.reserve( psInput.size() );

		for ( std::vector<CPsElementDesc>::iterator& iter = psInput.begin( ); iter != psInput.end( ); ++iter )
		{
			psOutput.emplace_back( iter->m_x, iter->m_y, iter->m_z, iter->m_color );
		}

		return psOutput;
	}
}