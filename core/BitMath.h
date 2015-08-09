#ifndef CORE_BITMATH_H
#define CORE_BITMATH_H

#include "core/RdeAssert.h"

namespace rde
{
// 'b'-th bit enabled.
#define RDE_BIT(b)				(1 << (b))
// Macro version for compile-time-asserts
#define RDE_IS_POWER_OF_TWO(x)	(((x) & ((x) - 1)) == 0)

inline bool IsPowerOfTwo(uint32_t x)
{
	return RDE_IS_POWER_OF_TWO(x);
}

// Undefined for x == 0.
// For x already being power of two returns x.
inline uint32_t NextPowerOfTwo(uint32_t x)
{
	RDE_ASSERT(x != 0);
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;	 
}

// Number of set bits
// Algorithm from Bit Twiddling Hacks (http://www-graphics.stanford.edu/~seander/bithacks.html)
inline uint32_t NumBits(uint32_t x)
{
  x -= (x >> 1) & 0x55555555;
  x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
  x = ((x >> 4) + x) & 0x0f0f0f0f;
  x += x >> 8;
  return (x + (x >> 16)) & 0x3f;
}

} // rde

#endif
