#pragma once
#include "CoolD_Type.h"
#include <assert.h>

void ClearColorBuffer( void* pImage, Duint width, Duint height, Dulong clearColor )
{
#define BUFFER_STRUCTURE( _base, _height, _width, _depth) *( _base + ( ( ( width * _height ) + _width ) * 3 ) + _depth )
	
	Duchar* buffer = (Duchar*)pImage;

	Duchar red	= (clearColor>>24);
	Duchar green	= (clearColor>>16) & 0x000000ff;
	Duchar blue	= (clearColor>>8 ) & 0x000000ff;
	Duchar currentcolor = 0;

	for ( Duint i = 0; i < height - 100; ++i) 
	{
		for (Duint j = 0; j < width- 100 ; ++j)
		{	
			for( Duint k = 0; k < 3; ++k)
			{
				if( k == 0 ) currentcolor = red;
				else if( k == 1 ) currentcolor = green;
				else if( k == 2 ) currentcolor = blue;				

				BUFFER_STRUCTURE(buffer, i, j, k) = currentcolor;
			}		
		}
	}
}

bool CheckContinue( const Line* lhs, const Line* rhs )
{
	if( lhs->beginIndex == rhs->endIndex ||	lhs->endIndex == rhs->beginIndex )
	{
		return true;
	}

	return false;
}