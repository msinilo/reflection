#if !defined (_MSC_VER)
#	error "MSVC C++ compiler required!"
#endif
#if !defined (CORE_CONFIG_H)
#	error "Do not include directly, only via core/Config.h"
#endif

#ifdef _DEBUG
#	undef RDE_DEBUG
#	define RDE_DEBUG	1
#endif

#define RDE_PLATFORM_WIN32	1
#define RDE_COMPILER_MSVC	1
#if defined(_M_IA64) || defined(_M_AMD64)
#	define RDE_64			1
#else
#	define RDE_64			0
#endif

// --- Wrappers over compiled specific declarations/keywords
#define RDE_FORCEINLINE		__forceinline
#define RDE_THREADLOCAL		__declspec(thread)
#define RDE_DLLIMPORT		__declspec(dllimport)
#define RDE_DLLEXPORT		__declspec(dllexport)
#define RDE_ALIGN(x)		__declspec(align(x))

// --- Test compiler options.
// Trick from ICE by Pierre Terdiman.
//#ifndef RDE_DONT_CHECK_COMPILER_OPTIONS
//#	define RDE_DONT_CHECK_COMPILER_OPTIONS
//#endif
#ifndef RDE_DONT_CHECK_COMPILER_OPTIONS
#	if defined (_CPPRTTI)
#		error "Please disable RTTI"
#	endif
#	if defined (_CPPUNWIND)
#		error "Please disable exceptions"
#	endif
#endif

// --- Warnings (enable additional)
#pragma warning(default: 4062)	// enum has no handler in case
#pragma warning(default: 4263)	// virtual function signature different in child class
#pragma warning(default: 4265)	// no virtual dtor
#pragma warning(default: 4431)	// missing type specifier
#pragma warning(default: 4545)	// ill-formed comma expressions
#pragma warning(default: 4546)  // function call before comma missing argument list
#pragma warning(default: 4547)  // operator before comma has no effect; expected operator with side-effect
#pragma warning(default: 4549)  // operator before comma has no effect; did you intend 'operator'?

// --- Intrinsics
#include <intrin.h>

#pragma intrinsic(abs)
#pragma intrinsic(memcmp, memcpy, memset)

// --- Types
namespace rde
{
	typedef signed char			int8_t;
	typedef unsigned char		uint8_t;
	typedef unsigned short		uint16_t;
	typedef signed short		int16_t;
	typedef unsigned long		uint32_t;
	typedef signed long			int32_t;
	typedef unsigned __int64	uint64_t;
	typedef signed __int64		int64_t;
#ifdef WIN64
	typedef unsigned __int64	PointerSizedUInt;
	typedef __int64				PointerSizedInt;
#else
	typedef unsigned int		PointerSizedUInt;
	typedef int					PointerSizedInt;
#endif

	RDE_ALIGN(16) struct MachineTypeWithStrictestAlignment
	{
		int member[4];
	};
} // rde
 