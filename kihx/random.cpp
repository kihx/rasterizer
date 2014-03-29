#include "stdafx.h"
#include "random.h"

#include <random>


namespace kih
{
	int Random::Next( int low, int high )
	{
#if _HAS_CPP0X
		// from http://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
		static std::random_device rd;			// only used once to initialize engine
		static std::mt19937 rng( rd() );		// random-number engine used
		std::uniform_int_distribution<int> uni( low, high ); // guaranteed unbiased
		return uni( rng );
#else
		// not implemented error!
#endif
	}
}
