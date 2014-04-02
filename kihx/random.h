#pragma once

#include "base.h"

namespace kih
{
	class Random final
	{
		NONCOPYABLE_CLASS( Random );

	public:
		static int Next( int low, int high );
	};
}

using kih::Random;
