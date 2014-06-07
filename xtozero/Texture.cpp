#include "stdafx.h"
#include "Texture.h"
#include "FileHandler.h"

#include <iostream>

namespace xtozero
{
	CFilter::CFilter()
	{

	}

	CFilter::~CFilter()
	{

	}

	CSampler::CSampler( const SAMPLER_DESC& samplerDesc ) :
		m_addressModeU( samplerDesc.m_addressModeU ),
		m_addressModeV( samplerDesc.m_addressModeV ),
		m_borderColor( samplerDesc.m_borderColor )
	{

	}
	CSampler::CSampler( const CSampler& sampler ) : m_addressModeU( sampler.m_addressModeU ),
	m_addressModeV( sampler.m_addressModeV ),
	m_borderColor( sampler.m_borderColor )
	{
		
	}
	CSampler::~CSampler( )
	{

	}

	inline void CSampler::CalcTextureAddress( Vector2& texcoord )
	{
		int integer = 0;

		switch ( m_addressModeU )
		{
		case TEXTURE_ADDRESS_WRAP:
			integer = static_cast<int>(texcoord.U);
			texcoord.U -= static_cast<float>(integer);
			if ( texcoord.U < 0.0f )
			{
				texcoord.U = 1.0f + texcoord.U;
			}
			break;
		case TEXTURE_ADDRESS_MIRROR:
			integer = static_cast<int>(texcoord.U );
			texcoord.U -= static_cast<float>(integer);
			if ( integer & 0x00000001 ) //홀수 일 경우
			{
				if ( integer >= 0 )
				{
					texcoord.U = 1.0f - texcoord.U;
				}
				else
				{
					texcoord.U = 1.0f + texcoord.U;
				}
			}
			else
			{
				if ( integer >= 0 )
				{
					//Do Nothing
				}
				else
				{
					texcoord.U = -texcoord.U;
				}
			}
			break;
		case TEXTURE_ADDRESS_CLAMP:
			if ( texcoord.U > 1.0f )
			{
				texcoord.U = 1.0f;
			}
			else if ( texcoord.U < 0.0f )
			{
				texcoord.U = 0.0f;
			}
			break;
		case TEXTURE_ADDRESS_BORDER:
			if ( texcoord.U > 1.0f || texcoord.U < 0.0f )
			{
				//유효하지 않은 좌표값을 설정.
				//유효하지 않은 좌표값일 경우 BORDER COLOR를 색상으로 하도록 한다.
				texcoord.U = 1.1f;
			}
			break;
		case TEXTURE_ADDRESS_MIRROR_ONCE:
			integer = static_cast<int>(texcoord.U);
			texcoord.U -= static_cast<float>(integer);
			if ( integer == 1 )
			{
				texcoord.U = 1.0f - texcoord.U;
			}
			else if ( integer >= 2 )
			{
				texcoord.U = 0.0f;
			}
			else if ( integer < -1 )
			{
				texcoord.U = 1.0f;
			}
			else if ( integer < 0 )
			{
				texcoord.U = -texcoord.U;
			}
			break;
		}

		switch ( m_addressModeV )
		{
		case TEXTURE_ADDRESS_WRAP:
			integer = static_cast<int>(texcoord.V);
			texcoord.V -= static_cast<float>(integer);
			if ( texcoord.V < 0.0f )
			{
				texcoord.V = 1.0f + texcoord.V;
			}
			break;
		case TEXTURE_ADDRESS_MIRROR:
			integer = static_cast<int>(texcoord.V);
			texcoord.V -= static_cast<float>(integer);
			if ( integer & 0x00000001 )
			{
				if ( integer >= 0 )
				{
					texcoord.V = 1.0f - texcoord.V;
				}
				else
				{
					texcoord.V = 1.0f + texcoord.V;
				}
			}
			else
			{
				if ( integer >= 0 )
				{
					//Do Nothing
				}
				else
				{
					texcoord.V = -texcoord.V;
				}
			}
			break;
		case TEXTURE_ADDRESS_CLAMP:
			if ( texcoord.V > 1.0f )
			{
				texcoord.V = 1.0f;
			}
			else if ( texcoord.V < 0.0f )
			{
				texcoord.V = 0.0f;
			}
			break;
		case TEXTURE_ADDRESS_BORDER:
			if ( texcoord.V > 1.0f || texcoord.V < 0.0f )
			{
				texcoord.V = 1.1f;
			}
			break;
		case TEXTURE_ADDRESS_MIRROR_ONCE:
			integer = static_cast<int>(texcoord.V);
			texcoord.V -= static_cast<float>(integer);
			if ( integer == 1 )
			{
				texcoord.V = 1.0f - texcoord.V;
			}
			else if ( integer >= 2 )
			{
				texcoord.V = 0.0f;
			}
			else if ( integer < -1 )
			{
				texcoord.V = 1.0f;
			}
			else if ( integer < 0 )
			{
				texcoord.V = -texcoord.V;
			}
			break;
		}
	}

	const unsigned int CSampler::GetBorderColor() const
	{
		return m_borderColor;
	}
	void CSampler::SetBorderColor( const int r, const int g, const int b )
	{
		m_borderColor = PIXEL_COLOR( r, g, b );
	}

	CTexture::CTexture( const unsigned int width, const unsigned int height )
		: m_width( width ), m_height( height )
	{
		assert( width > 0 && height > 0 );
		m_texture.resize( width * height );
	}

	CTexture::CTexture()
	{
	}

	CTexture::~CTexture()
	{
	}

	void CTexture::SetSize( const unsigned int width, const unsigned int height )
	{
		m_width = width;
		m_height = height;
		m_texture.resize( width * height );
	}

	CBitmap::CBitmap( const unsigned int width, const unsigned int height ) : CTexture( width, height )
	{

	}

	CBitmap::CBitmap( ) 
	{

	}

	CBitmap::~CBitmap( )
	{

	}

	void CBitmap::Load( const char *pfileName )
	{
		CFileHandler bitmapFile( pfileName, std::ios::in | std::ios::binary );

		if ( bitmapFile.is_open() )
		{
			bitmapFile.read( reinterpret_cast<char*>(&m_bitmapHeader), sizeof(BITMAPFILEHEADER) );
			bitmapFile.read( reinterpret_cast<char*>(&m_bitmapInfoHeader), sizeof(BITMAPINFOHEADER) );

			if ( m_bitmapHeader.bfType == 0x4D42 ) // 'BM'의 아스키 코드. 이걸로 비트맵인지 여부를 알 수 있다.
			{
				if ( m_bitmapInfoHeader.biBitCount > 8 )
				{
					//바로 픽셀 정보가 나옴 

					SetSize( m_bitmapInfoHeader.biWidth, m_bitmapInfoHeader.biHeight );

					if ( m_bitmapInfoHeader.biBitCount == 24 )
					{
						//상하 반전 되어 있으므로 거꾸로 읽는다.
						for ( int i = m_bitmapInfoHeader.biHeight - 1; i >= 0; --i )
						{
							for ( int j = 0; j < m_bitmapInfoHeader.biWidth; ++j )
							{
								int index = i * m_bitmapInfoHeader.biWidth + j;
								bitmapFile.read( reinterpret_cast<char*>(&m_texture[index].m_color[BLUE]), sizeof(unsigned char) );
								bitmapFile.read( reinterpret_cast<char*>(&m_texture[index].m_color[GREEN]), sizeof(unsigned char) );
								bitmapFile.read( reinterpret_cast<char*>(&m_texture[index].m_color[RED]), sizeof(unsigned char) );// bmp는 bgr 순서로 저장되어 있다.
							}
						}
					}
				}
				else
				{
					//8 비트 아래 부터는 파레트 정보가 먼저 나옴
				}
			}
			else
			{
				std::cout << "BMP 파일이 아님" << std::endl;
			}
		}
	}

	CTextureManager::CTextureManager( )
	{

	}

	CTextureManager::~CTextureManager( )
	{

	}

	std::shared_ptr<CTexture> CTextureManager::Load( const char *pfileName )
	{
		Lock<SpinLock> lock( m_lockObject );

		std::string fileName( pfileName );

		std::map<std::string, std::shared_ptr<CTexture> >::iterator findedTex = m_textureMap.find( fileName );

		if ( findedTex == m_textureMap.end() )
		{
			const char* exten = nullptr;

			exten = strrchr( pfileName, '.' );
			if ( exten )
			{
				if ( strncmp( exten, ".bmp", 4 ) == 0 )
				{
					std::shared_ptr<CTexture> texture( new CBitmap( ) );

					texture->Load( pfileName );

					m_textureMap.insert( make_pair( fileName, texture ) );

					return texture;
				}
				else
				{
					return nullptr;
				}
			}
			else
			{
				return nullptr;
			}
		}
		else
		{
			return findedTex->second;
		}
	}

	const unsigned int CBitmap::Sample( const float u, const float v, std::shared_ptr<CSampler> sampler ) const
	{
		Vector2 texCoord( u, v );

		sampler->CalcTextureAddress( texCoord );

		if ( texCoord.U > 1.0f  || texCoord.V > 1.0f  )
		{
			return sampler->GetBorderColor();
		}

		int texX = static_cast<int>(texCoord.U * (m_bitmapInfoHeader.biWidth - 1));
		int texY = static_cast<int>(texCoord.V * (m_bitmapInfoHeader.biHeight - 1));

		int index = texY * m_bitmapInfoHeader.biWidth + texX;

		return PIXEL_COLOR( m_texture[index].m_color[RED],
							m_texture[index].m_color[GREEN],
							m_texture[index].m_color[BLUE] );
	}

	const unsigned int CBitmap::Sample( const Vector2& texCoord, std::shared_ptr<CSampler> sampler ) const
	{
		Vector2 _texCoord( texCoord );
		
		sampler->CalcTextureAddress( _texCoord );

		int texX = static_cast<int>(_texCoord.U * (m_bitmapInfoHeader.biWidth - 1));
		int texY = static_cast<int>(_texCoord.V * (m_bitmapInfoHeader.biHeight - 1));

		if ( _texCoord.U > 1.0f || _texCoord.V > 1.0f )
		{
			return sampler->GetBorderColor( );
		}

		unsigned int index = texY * m_bitmapInfoHeader.biWidth + texX;

		return PIXEL_COLOR( m_texture[index].m_color[RED],
							m_texture[index].m_color[GREEN],
							m_texture[index].m_color[BLUE] );
	}

	void CBitmap::DrawTexture( void* buffer, int width, int dpp ) const
	{
		size_t size = dpp / 8;

		unsigned char* frameBuffer = static_cast<unsigned char*>(buffer);

		for ( int i = 0; i < m_bitmapInfoHeader.biWidth; ++i )
		{
			for ( int j = 0; j < m_bitmapInfoHeader.biHeight; ++j )
			{
				unsigned char* pixel = frameBuffer + ((width * j) + i) * size;
				int index = j * m_bitmapInfoHeader.biWidth + i;
				pixel[0] = m_texture[index].m_color[RED];
				pixel[1] = m_texture[index].m_color[GREEN];
				pixel[2] = m_texture[index].m_color[BLUE];
			}
		}
	}
}