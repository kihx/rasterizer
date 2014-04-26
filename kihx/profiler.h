#pragma once

#include "base.h"


#define PROFILE_LEVEL	0


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
		ScopeProfile( const char* name = nullptr ) :
			m_beginTime( PlatformTime::MicroSeconds() ),
			m_name( name )
		{
		}

		~ScopeProfile();

	private:
		double m_beginTime;
		const char* m_name;
	};
};

using kih::PlatformTime;
using kih::ScopeProfile;



#if PROFILE_LEVEL <= 0

#define SCOPE_PROFILE_BEGIN( name )
#define SCOPE_PROFILE_END()

#else

#define SCOPE_PROFILE_BEGIN( name )	\
	{ ScopeProfile __profile( name );
#define SCOPE_PROFILE_END()		}

#endif	// PROFILE_LEVEL
