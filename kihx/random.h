#pragma once

#include "base.h"

namespace kih
{
	class Random
	{
		NONCOPYABLE_CLASS( Random );

	public:
		static int Next( int low, int high );
	};
}

using kih::Random;
