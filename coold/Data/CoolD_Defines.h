#pragma once

//------------Output FileName & LineNumber Show-----------------
#define STR2(x) #x
#define STR(x) STR2(x)
#define MSG(desc) message(__FILE__ "(" STR(__LINE__) "):" #desc)
#define FixLater(desc) __pragma(MSG(desc))
//-------------------------------------------------------------- 

#define RED		0xff0000ff
#define GREEN	0x00ff00ff
#define BLUE	0x0000ffff
#define WHITE	0xffffffff
#define BLACK	0x000000ff

//Singleton
#define GETSINGLE(p)	p::GetInstance()
#define DESTROY(p)		p::DestroyInstance()

//¹Ý¿Ã¸²
#define ROUND_Off(x, dig) (Dfloat)(floor((x) * pow(10,dig) + 0.5) / pow(10,dig))

//std macro
#define STD_FIND_IF(type, func) find_if(type.begin(), type.end(), func)
#define STD_REMOVE_IF(type, func) remove_if(type.begin(), type.end(), func)
#define STD_ERASE(type, iter) type.erase(iter, type.end())

//IterType
#define ITER_CONVERT(Iter, IteratorValue) auto* Iter = IteratorValue;

#define LOG( msg )	{ printf( "[Log] %s\n", msg); }


