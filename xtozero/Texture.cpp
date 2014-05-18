#include "stdafx.h"
#include "Texture.h"
#include "FileHandler.h"

#include <iostream>

namespace xtozero
{
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
								bitmapFile >> m_texture[index].m_color[BLUE]
									>> m_texture[index].m_color[GREEN]
									>> m_texture[index].m_color[RED]; // bmp는 bgr 순서로 저장되어 있다.
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
}