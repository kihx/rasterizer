#include "stdafx.h"
#include "PixelShader.h"

namespace xtozero
{
	const float INV_PI = 1 / 3.1415926535897f;

	Vector2 CalcShpericalTexCoord( const Vector3& vertex )
	{
		//Local 좌표에서 UV 값을 계산.

		Vector3 normalizeVertex = vertex;

		normalizeVertex.Normalize( );

		//http://en.wikipedia.org/wiki/UV_mapping#Finding_UV_on_a_sphere
		return Vector2( 0.5f + (atan2( normalizeVertex.Z, normalizeVertex.X ) * (0.5f * INV_PI))
			, 0.5f - (asin( normalizeVertex.Y ) * INV_PI) );
	}

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
			Vector3 localVertex = ominput.m_localVertex;
			localVertex.Normalize();
			unsigned int color = m_textures[0]->GetTexel( CalcShpericalTexCoord( localVertex ) );
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