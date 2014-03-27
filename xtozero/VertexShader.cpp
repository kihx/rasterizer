#include "stdafx.h"
#include "VertexShader.h"

namespace xtozero
{
	CVertexShader::CVertexShader()
	{
	}


	CVertexShader::~CVertexShader()
	{
	}

	std::vector<CRsElementDesc> CVertexShader::Process( const std::shared_ptr<CMesh> pMesh )
	{
		std::vector<CRsElementDesc> vsOutput;
		return vsOutput;
	}
}
