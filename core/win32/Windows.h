// Include this file instead of <windows.h>
// It deals with some of the most annoying problems
// (mainly weird macros in windows.h)
#ifndef CORE_WIN32_WINDOWS_H
#define CORE_WIN32_WINDOWS_H

#ifdef _WINDOWS
#	error "<windows.h> already included!"
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#endif 
