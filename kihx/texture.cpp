#include "stdafx.h"
#include "texture.h"


namespace kih
{
	/* class Texture
	*/
	std::shared_ptr<Texture> Texture::Create( int width, int height, ColorFormat format, void* pExternalMemory )
	{
		// Cannot use std::make_shared<>() by the protected access modifier.
		auto texture = std::shared_ptr<Texture>( new Texture() );
		texture->m_width = width;
		texture->m_height = height;
		texture->m_format = format;
		texture->m_flags = 0;

		if ( pExternalMemory )
		{
			texture->SetExternalMemory( pExternalMemory );
		}
		else
		{
			texture->m_pMemory = malloc( width * height * GetBytesPerPixel( format ) );
		}

		return texture;
	}
};
