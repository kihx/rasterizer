#include "Utility.h"

int Utility::Float2Int(float f)
{
#ifdef _WIN32
	int i;
	__asm
	{
		fld  f
			fistp i
	}
	return i;
#else
	return (int)(f + 0.5f);
#endif 
}

Matrix4 Utility::Float2Matrix4(const float* mat)
{
	return Matrix4(mat[0], mat[1], mat[2], mat[3],
		mat[4], mat[5], mat[6], mat[7],
		mat[8], mat[9], mat[10], mat[11],
		mat[12], mat[13], mat[14], mat[15]);
}