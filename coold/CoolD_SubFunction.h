#pragma once

void ClearColorBuffer( void* pImage, int width, int height, unsigned long clearColor )
{
#define BUFFER_STRUCTURE( _base, _height, _width, _depth) *( _base + ( ( ( width * _height ) + _width ) * 3 ) + _depth )

	unsigned char* buffer = (unsigned char*)pImage;

	unsigned char red	= (clearColor>>24);
	unsigned char green	= (clearColor>>16) & 0x000000ff;
	unsigned char blue	= (clearColor>>8 ) & 0x000000ff;
	unsigned char currentcolor = 0;

	for (int i = 0; i < height ; ++i) 
	{
		for (int j = 0; j < width; ++j)
		{	
			for( int k = 0; k < 3; ++k)
			{
				if( k == 0 ) currentcolor = red;
				else if( k == 1 ) currentcolor = green;
				else if( k == 2 ) currentcolor = blue;				

				BUFFER_STRUCTURE(buffer, i, j, k) = currentcolor;
			}		
		}
	}
}