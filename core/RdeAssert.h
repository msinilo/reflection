#ifndef CORE_RDEASSERT_H
#define CORE_RDEASSERT_H

#include "core/Config.h"	// for RDE_DEBUG

namespace rde
{
namespace Assertion
{
	// @return true if handled/false if not (will halt!)
	typedef bool (*Handler)(const char* expr, const char* file, int line);
	Handler SetHandler(Handler newHandler);

	// @internal Called when assertion fails.
	bool Failure(const char* expr, const char* file, int line);
} // Assertion

#define RDE_HALT()	do { __debugbreak(); } while (0) 

// Base assert macro. Will work even in release builds.
// @see RDE_ASSERT
#define RDE_ASSERT_ALWAYS(expr)										\
	do																\
	{																\
		if (!(expr))												\
		{															\
			if (!rde::Assertion::Failure("Assertion failed: "#expr,	\
				__FILE__, __LINE__))								\
			{														\
				RDE_HALT();											\
			}														\
		}															\
	}																\
	while (false)

#ifndef RDE_ASSERT_ENABLED
#	define RDE_ASSERT_ENABLED	RDE_DEBUG
#endif

#if RDE_ASSERT_ENABLED && !RDE_DEBUG
#	ifdef WIN32
#		pragma message ("Assertions enabled in a non-debug build")
#	endif
#endif

#if RDE_ASSERT_ENABLED
#	define RDE_ASSERT(expr)				RDE_ASSERT_ALWAYS(expr)
#	define RDE_ASSUME(expr, if_fails)	RDE_ASSERT_ALWAYS(expr)
#	define RDE_VERIFY(expr)				RDE_ASSERT_ALWAYS(expr)
#else
#	define RDE_ASSERT(expr)	do { (void)sizeof(expr); } while (0)
#	define RDE_ASSUME(expr, if_fails)	if (!(expr)) { if_fails; }
#	define RDE_VERIFY(expr)	expr
#endif

#if RDE_PEDANTIC
#	define RDE_ASSERT_PEDANTIC(expr)	RDE_ASSERT_ALWAYS(expr)
#else
#	define RDE_ASSERT_PEDANTIC(expr)	do { (void)sizeof(expr); } while (0)
#endif

// Compile time assert, causes compile error if !expr.
#define RDE_COMPILE_CHECK(expr)	typedef char CC_##__LINE__ [(expr) ? 1 : -1]

} // rde

#endif
 