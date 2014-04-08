#pragma once
#include "..\Data\CoolD_Type.h"

Dvoid ClearColorBuffer( Dvoid* pImage, Duint width, Duint height, Dulong clearColor )
{
	Duchar* buffer = (Duchar*)pImage;

	Duchar red	 = (clearColor>>24);
	Duchar green = (clearColor>>16) & 0x000000ff;
	Duchar blue	 = (clearColor>>8 ) & 0x000000ff;
	Duchar currentcolor = 0;

	for ( Duint i = 0; i < height; ++i) 
	{
		for (Duint j = 0; j < width; ++j)
		{	
			for( Duint k = 0; k < 3; ++k)
			{
				if( k == 0 ) currentcolor = red;
				else if( k == 1 ) currentcolor = green;
				else if( k == 2 ) currentcolor = blue;				

				*(buffer + (((width * i) + j) * 3) + k) = currentcolor;
			}		
		}
	}
}
