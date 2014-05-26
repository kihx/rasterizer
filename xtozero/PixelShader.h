#ifndef _PIXELSHADER_H_
#define _PIXELSHADER_H_

#include "pipelineElements.h"
#include "Texture.h"

namespace xtozero
{
	const int MAX_TEXTURE = 8;

	class CPixelShader
	{
	private:
		std::vector<COmElementDesc> m_psOutput;
		
		CTexture* m_textures[MAX_TEXTURE];
	public:
		CPixelShader( );
		~CPixelShader( );
		const std::vector<COmElementDesc>& Process( const std::vector<CPsElementDesc>& psInput );

		void PSSetTexture( const int index, CTexture* texture );
	};
}

#endif

