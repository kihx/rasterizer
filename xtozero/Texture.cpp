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

			if ( m_bitmapHeader.bfType == 0x4D42 ) // 'BM'�� �ƽ�Ű �ڵ�. �̰ɷ� ��Ʈ������ ���θ� �� �� �ִ�.
			{
				if ( m_bitmapInfoHeader.biBitCount > 8 )
				{
					//�ٷ� �ȼ� ������ ���� 

					SetSize( m_bitmapInfoHeader.biWidth, m_bitmapInfoHeader.biHeight );

					if ( m_bitmapInfoHeader.biBitCount == 24 )
					{
						//���� ���� �Ǿ� �����Ƿ� �Ųٷ� �д´�.
						for ( int i = m_bitmapInfoHeader.biHeight - 1; i >= 0; --i )
						{
							for ( int j = 0; j < m_bitmapInfoHeader.biWidth; ++j )
							{
								int index = i * m_bitmapInfoHeader.biWidth + j;
								bitmapFile >> m_texture[index].m_color[BLUE]
									>> m_texture[index].m_color[GREEN]
									>> m_texture[index].m_color[RED]; // bmp�� bgr ������ ����Ǿ� �ִ�.
							}
						}
					}
				}
				else
				{
					//8 ��Ʈ �Ʒ� ���ʹ� �ķ�Ʈ ������ ���� ����
				}
			}
			else
			{
				std::cout << "BMP ������ �ƴ�" << std::endl;
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