#ifndef _PIXELSHADER_H_
#define _PIXELSHADER_H_

#include "pipelineElements.h"

namespace xtozero
{
	class CPixelShader
	{
	public:
		CPixelShader( );
		~CPixelShader( );
		std::vector<COmElementDesc> Process( std::vector<CPsElementDesc>& psInput );
	};
}

#endif

