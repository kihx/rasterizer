#pragma once
#include "CoolD_Type.h"
#include <assert.h>

void ClearColorBuffer( void* pImage, Duint width, Duint height, Dulong clearColor )
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

void DotRender(void* pImage, const Duint width, const Duint height, const Duint x, const Duint y, const Dulong clearColor)
{
	Duchar* buffer = (Duchar*)pImage;

	Duchar red	 = (clearColor >> 24);
	Duchar green = (clearColor >> 16) & 0x000000ff;
	Duchar blue  = (clearColor >> 8 ) & 0x000000ff;
	Duchar currentcolor = 0;
	
	for( Duint k = 0; k < 3; ++k )
	{
		if( k == 0 ) currentcolor = red;
		else if( k == 1 ) currentcolor = green;
		else if( k == 2 ) currentcolor = blue;

		*(buffer + (((width * y) + x) * 3) + k) = currentcolor;
	}	
}

Dulong MixDotColor(const BaseColor& color)
{
	return (color.r << 24) + (color.g << 16) + (color.b << 8) + (color.a);
}

bool CheckContinueLine(const Line* lhs, const Line* rhs)
{	//¾Æ..±¸·Á....
	if( lhs->lineKey.beginIndex == rhs->lineKey.beginIndex	||
		lhs->lineKey.beginIndex == rhs->lineKey.endIndex	||
		lhs->lineKey.endIndex == rhs->lineKey.beginIndex	||
		lhs->lineKey.endIndex == rhs->lineKey.beginIndex )
	{
		return true;
	}

	return false;
}