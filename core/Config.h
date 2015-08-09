#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#if defined (_MSC_VER)
#	include "msvc/MsvcConfig.h"
#else
#	error "Compiler not supported"
#endif

#define RDE_NEEDS_MINIDUMP	1

#ifndef RDE_REFCOUNT_TRACK
#	define RDE_REFCOUNT_TRACK	RDE_DEBUG
#endif

// Makes copy constructor+assignment operator private.
#define RDE_FORBID_COPY(CLASS)	\
	private: CLASS(const CLASS&); CLASS& operator=(const CLASS&)

namespace rde
{
// Number of elements in array.
// Safer than sizeof(array)/sizeof(array[0]) because it wont accept pointers.
template<typename T, size_t N> char (&ArrayCountObj(const T (&)[N]))[N];
#define RDE_ARRAY_COUNT(arr)	(sizeof(rde::ArrayCountObj(arr)))
}

#endif // #ifndef CORE_CONFIG_H

