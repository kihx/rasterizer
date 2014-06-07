#ifndef _PIXELSHADER_H_
#define _PIXELSHADER_H_

#include "pipelineElements.h"
#include "Texture.h"

namespace xtozero
{
	const int MAX_TEXTURE = 8;
	const int MAX_SAMPLER = 8;

	class CPixelShader
	{
	private:
		std::vector<COmElementDesc> m_psOutput;
		
		std::shared_ptr<CTexture> m_textures[MAX_TEXTURE];
		std::shared_ptr<CSampler> m_samplers[MAX_SAMPLER];

	public:
		CPixelShader( );
		~CPixelShader( );
		const std::vector<COmElementDesc>& Process( const std::vector<CPsElementDesc>& psInput );

		void PSSetTexture( const int index, std::shared_ptr<CTexture> texture );
		void PSSetSampler( const int index, std::shared_ptr<CSampler> sampler );
	};
}

#endif

