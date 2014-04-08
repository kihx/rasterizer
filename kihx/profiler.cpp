#include "stdafx.h"
#include "profiler.h"

#include <windows.h>


namespace kih
{
	double PlatformTime::FrequencyCycle = 0;

	double PlatformTime::MicroSeconds()
	{
		static bool TimeInitialized = false;
		if ( !TimeInitialized )
		{
			TimeInitialized = true;
		
			LARGE_INTEGER perfFrequency;
			QueryPerformanceFrequency( &perfFrequency );
			FrequencyCycle = 1.0 / perfFrequency.QuadPart;
		}

		LARGE_INTEGER counter;
		QueryPerformanceCounter( &counter );
		return counter.QuadPart * FrequencyCycle * 1000000;
	}
}
