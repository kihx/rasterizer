#define DLLEXPORTS
//#include "Windows.h"
#include "coold.h"

//BOOL WINAPI DllMain( HINSTANCE hInstDll, DWORD fdwReason, LPVOID fImpLoad )
//{
//	return TRUE;
//}

EXTERN_FORM DLL_API void __cdecl colorControl( void* pImage, int width, int height, unsigned long clearColor )
{
	unsigned char* buffer = (unsigned char*)pImage;

	int red		= clearColor>>24;
	int green	= clearColor>>16;
	int blue	= clearColor>>8;	
	
	for (int i = 0; i < height ; i++) 
	{
		for (int j = 0; j < width; j++)
		{				
			*( buffer + ( ( ( width * i ) + j ) * 3 ) + 0 ) = red;
			*( buffer + ( ( ( width * i ) + j ) * 3 ) + 1 ) = green;
			*( buffer + ( ( ( width * i ) + j ) * 3 ) + 2 ) = blue;
		}					
	}		
}

