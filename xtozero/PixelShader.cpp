#include "stdafx.h"
#include "PixelShader.h"

namespace xtozero
{
	CPixelShader::CPixelShader()
	{
		for ( int i = 0; i < MAX_TEXTURE; ++i )
		{
			m_textures[i] = nullptr;
		}
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
			unsigned int color = m_textures[0]->GetTexel( ominput.m_texCoord.GetU(),
														ominput.m_texCoord.GetV() );
			m_psOutput[i].m_x = ominput.m_x;
			m_psOutput[i].m_y = ominput.m_y;
			m_psOutput[i].m_z = ominput.m_z;
			m_psOutput[i].m_color = color;
		}

		return m_psOutput;
	}

	void CPixelShader::PSSetTexture( const int index, CTexture* texture )
	{
		m_textures[index] = texture;
	}
}