#pragma once

class WTriData;
class WPolyData;

namespace FileLoader
{
	WTriData* LoadMesh(const char* fileName);
	WPolyData* LoadPoly(const char* fileName);
}