#pragma once

/* Standard C++ library supports
*/
#include <vector>


#define ALLOCATOR_OVERRIDE


#ifdef ALLOCATOR_OVERRIDE

template<class T>
using StlVector = std::vector<T, kih::StlAllocator<T>>;

#else

template<class T>
using StlVector = std::vector<T>;

#endif	// ALLOCATOR_OVERRIDE

