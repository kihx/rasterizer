#pragma once

#include "base.h"

#include <stdio.h>


namespace kih
{
	/* class PlatformTime
	*/
	class PlatformTime
	{
	public:
		static double MicroSeconds();

	private:
		static double FrequencyCycle;
	};


	/* class PlatformTime
	*/
	class ScopeProfile
	{
	public:
		ScopeProfile() :
			m_beginTime( PlatformTime::MicroSeconds() )
		{
		}

		~ScopeProfile()
		{
			printf( "%.2f\n", ( PlatformTime::MicroSeconds() - m_beginTime ) / 1000.0 );
		}

	private:
		double m_beginTime;
	};
};

using kih::PlatformTime;
using kih::ScopeProfile;
