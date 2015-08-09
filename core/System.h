#ifndef CORE_SYSTEM_H
#define CORE_SYSTEM_H

#include "core/Config.h"

namespace rde
{
namespace Sys
{
	void MemCpy(void* to, const void* from, size_t bytes);
	void MemMove(void* to, const void* from, size_t bytes);
	void MemSet(void* buf, uint8_t value, size_t bytes);
} // Sys
}

#if RDE_PLATFORM_WIN32
#	include "win32/Win32System.h"
#else
#	error "Platform not supported"
#endif

#endif // #ifndef CORE_SYSTEM_H
