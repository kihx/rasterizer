#ifndef _PIXELSHADER_H_
#define _PIXELSHADER_H_

#include "pipelineElements.h"

namespace xtozero
{
	class CPixelShader
	{
	private:
		std::vector<COmElementDesc> m_psOutput;
	public:
		CPixelShader( );
		~CPixelShader( );
		const std::vector<COmElementDesc>& Process( const std::vector<CPsElementDesc>& psInput );
	};
}

#endif

