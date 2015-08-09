#include "core/RdeAssert.h"

namespace
{
rde::Assertion::Handler s_handler(0);
int						s_numAssertionFailures(0);
}

namespace rde
{
Assertion::Handler Assertion::SetHandler(Handler newHandler)
{
	Handler prevHandler = s_handler;
	s_handler = newHandler;
	return prevHandler;
}

bool Assertion::Failure(const char* expr, const char* file, int line)
{
	++s_numAssertionFailures;
	if (s_handler)
		return s_handler(expr, file, line);
	return false;
}
} // rde
