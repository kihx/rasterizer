#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <vector>
#include <map>
#include <string>
#include <memory>
#include "XtzMath.h"
#include "XtzThreadPool.h"

namespace xtozero
{
	enum COLOR
	{
		BLUE = 0,
		GREEN,
		RED,
	};

	struct TEXEL
	{
		unsigned char m_color[3];
	};

	class CFilter
	{
	public:
		explicit CFilter();
		~CFilter();
	};

	enum TEXTURE_ADDRESS_MODE
	{
		TEXTURE_ADDRESS_WRAP = 1,
		TEXTURE_ADDRESS_MIRROR,
		TEXTURE_ADDRESS_CLAMP,
		TEXTURE_ADDRESS_BORDER,
		TEXTURE_ADDRESS_MIRROR_ONCE,
	};

	struct SAMPLER_DESC
	{
	public:
		TEXTURE_ADDRESS_MODE m_addressModeU;
		TEXTURE_ADDRESS_MODE m_addressModeV;
		unsigned int m_borderColor;
	};

	class CSampler
	{
	private:
		std::shared_ptr<CFilter> m_filter;

		TEXTURE_ADDRESS_MODE m_addressModeU;
		TEXTURE_ADDRESS_MODE m_addressModeV;

		unsigned int m_borderColor;
	public:
		explicit CSampler( const SAMPLER_DESC& samplerDesc );
		CSampler( const CSampler& sampler );
		~CSampler( );

		inline void CalcTextureAddress( Vector2& texcoord );
		inline const unsigned int GetBorderColor() const;
		inline void SetBorderColor( const int r, const int g, const int b );
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
		virtual const unsigned int  Sample( const float u, const float v, std::shared_ptr<CSampler> sampler ) const = 0;
		virtual const unsigned int  Sample( const Vector2& texCoord, std::shared_ptr<CSampler> sampler ) const = 0;
		virtual void DrawTexture( void* buffer, int width, int dpp ) const = 0;
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
		virtual const unsigned int  Sample( const float u, const float v, std::shared_ptr<CSampler> sampler ) const;
		virtual const unsigned int  Sample( const Vector2& texCoord, std::shared_ptr<CSampler> sampler ) const;
		virtual void DrawTexture( void* buffer, int width, int dpp ) const;
	};

	class CTextureManager
	{
	private:
		SpinLock m_lockObject;
		std::map<std::string, std::shared_ptr<CTexture> > m_textureMap;
	public:
		explicit CTextureManager( );
		~CTextureManager( );

		std::shared_ptr<CTexture> Load( const char *pfileName );
	};
}

#endif