#include "stdafx.h"
#include "Concommand.h"
#include "PixelShader.h"

namespace xtozero
{
	const float INV_PI = 1 / 3.1415926535897f;

	cmd::CConvar g_colorMode( "colorMode", "2" );

	cmd::CConvar g_solidRed( "solidRed", "255" );
	cmd::CConvar g_solidBlue( "solidBlue", "255" );
	cmd::CConvar g_solidGreen( "solidGreen", "255" );

	cmd::CConvar g_toggleShpericalMapping( "useShperical", "1" );

	cmd::CConvar g_texcoordUScale( "uScale", "1" );
	cmd::CConvar g_texcoordVScale( "vScale", "1" );

	DECLARE_CONCOMMAND( setColor )
	{
		int arg = cmd::CConcommandExecutor::GetInstance().ArgC();

		if ( arg == 4 )
		{
			g_solidRed.SetValue( cmd::CConcommandExecutor::GetInstance().ArgV( 1 ) );
			g_solidBlue.SetValue( cmd::CConcommandExecutor::GetInstance().ArgV( 2 ) );
			g_solidGreen.SetValue( cmd::CConcommandExecutor::GetInstance().ArgV( 3 ) );
		}
	}

	const Vector2 CalcShpericalTexCoord( const Vector3& vertex )
	{
		//Local 좌표에서 UV 값을 계산.

		Vector3 normalizeVertex = vertex;

		normalizeVertex.Normalize( );

		//http://en.wikipedia.org/wiki/UV_mapping#Finding_UV_on_a_sphere
		return Vector2( (0.5f + (atan2( normalizeVertex.Z, normalizeVertex.X ) * (0.5f * INV_PI))) * g_texcoordUScale.GetFloat()
			, (0.5f - (asin( normalizeVertex.Y ) * INV_PI)) * g_texcoordVScale.GetFloat() );
	}

	const Vector2 CalcCylindericalTexCoord( const Vector3& vertex )
	{
		//Local 좌표에서 UV 값을 계산.

		Vector3 normalizeVertex = vertex;

		float y = normalizeVertex.Y;

		normalizeVertex.Normalize( );

		return Vector2( (0.5f + (atan2( normalizeVertex.Z, normalizeVertex.X ) * (0.5f * INV_PI))) * g_texcoordUScale.GetFloat()
			, ( (y / 0.3f) * -0.5f + 0.5f ) * g_texcoordVScale.GetFloat( ) );
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
			unsigned int color = 0;
			const CPsElementDesc& ominput = psInput[i];
			if ( g_colorMode.GetInt() == 0 )
			{
				color = RAND_COLOR();
			}
			else if ( g_colorMode.GetInt() == 1 )
			{
				color = PIXEL_COLOR( g_solidRed.GetInt( ), g_solidBlue.GetInt( ), g_solidGreen.GetInt() );
			}
			else if ( g_colorMode.GetInt() == 2 )
			{
				Vector3 localVertex = ominput.m_localVertex;

				if ( g_toggleShpericalMapping.GetBool() )
				{
					color = m_textures[0]->Sample( CalcShpericalTexCoord( localVertex ),
													m_samplers[0]);
				}
				else
				{
					color = m_textures[0]->Sample( CalcCylindericalTexCoord( localVertex ),
													m_samplers[0] );
				}
			}
			m_psOutput[i].m_x = ominput.m_x;
			m_psOutput[i].m_y = ominput.m_y;
			m_psOutput[i].m_z = ominput.m_z;
			m_psOutput[i].m_color = color;
		}

		return m_psOutput;
	}

	void CPixelShader::PSSetTexture( const int index, std::shared_ptr<CTexture> texture )
	{
		if ( index < MAX_TEXTURE )
		{
			m_textures[index] = texture;
		}
	}

	void CPixelShader::PSSetSampler( const int index, std::shared_ptr<CSampler> sampler )
	{
		if ( index < MAX_SAMPLER )
		{
			m_samplers[index] = sampler;
		}
	}
}