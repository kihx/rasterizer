#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <vector>
#include <map>
#include <string>
#include <memory>
#include "XtzMath.h"

namespace xtozero
{
	enum color
	{
		BLUE = 0,
		GREEN,
		RED,
	};

	struct TEXEL
	{
		unsigned char m_color[3];
	};

	class CTexture
	{
	private:
		unsigned int		m_width;
		unsigned int		m_height;
	protected:
		std::vector<TEXEL>	m_texture;

		const unsigned int GetWidth() const
		{
			return m_width;
		}

		const unsigned int GetHeight() const
		{
			return m_height;
		}
	public:
		CTexture( const unsigned int width, const unsigned int height );
		CTexture( );
		virtual ~CTexture( );

		void SetSize( const unsigned int width, const unsigned int height );

		virtual void Load( const char *pfileName ) = 0;
		virtual unsigned int  GetTexel( const float u, const float v ) = 0;
		virtual unsigned int  GetTexel( const Vector2& texCoord ) = 0;
		virtual void DrawTexture( void* buffer, int width, int height, int dpp ) = 0;
	};

	class CBitmap : public CTexture
	{
	private:
		BITMAPFILEHEADER	m_bitmapHeader;
		BITMAPINFOHEADER	m_bitmapInfoHeader;
	public:
		CBitmap( const unsigned int width, const unsigned int height );
		CBitmap( );
		virtual ~CBitmap( );

		virtual void Load( const char *pfileName );
		virtual unsigned int  GetTexel( const float u, const float v );
		virtual unsigned int  GetTexel( const Vector2& texCoord );
		virtual void DrawTexture( void* buffer, int width, int height, int dpp );
	};

	class CTextureManager
	{
	private:
		std::map<std::string, std::shared_ptr<CTexture> > m_textureMap;
	public:
		CTextureManager( );
		~CTextureManager( );

		std::shared_ptr<CTexture> Load( const char *pfileName );
	};
}

#endif