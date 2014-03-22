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

//¹Ý¿Ã¸²
#define RoundOff(x, dig) (float)(floor((x) * pow(10,dig) + 0.5) / pow(10,dig))
