#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <vector>
#include <map>
#include <string>
#include <memory>

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
	public:
		CTexture( const unsigned int width, const unsigned int height );
		CTexture( );
		virtual ~CTexture( );

		void SetSize( const unsigned int width, const unsigned int height );

		virtual void Load( const char *pfileName ) = 0;
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