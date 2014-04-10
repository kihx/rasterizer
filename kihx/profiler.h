#pragma once

#include "base.h"


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

		~ScopeProfile();

	private:
		double m_beginTime;
	};
};

using kih::PlatformTime;
using kih::ScopeProfile;
