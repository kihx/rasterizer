// kihx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "kihx.h"


// This is an example of an exported variable
KIHX_API int nkihx=0;

// This is an example of an exported function.
KIHX_API int fnkihx(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see kihx.h for the class definition
Ckihx::Ckihx()
{
	return;
}
