#include "stdafx.h"
#include "concommand.h"
#include "profiler.h"

#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


namespace kih
{
	/* class PlatformTime
	*/
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


	/* class ScopeProfile
	*/
	ScopeProfile::~ScopeProfile()
	{
		if ( m_name )
		{
			std::cout << m_name << ": ";
		}

		std::cout << ( PlatformTime::MicroSeconds() - m_beginTime ) / 1000.0 << std::endl;
	}
}
