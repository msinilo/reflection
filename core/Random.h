#ifndef CORE_RANDOM_H
#define CORE_RANDOM_H

#define RDE_NEEDS_MERSENNE_TWISTER_RNG	0

namespace rde
{
#if RDE_NEEDS_MERESENNE_TWISTER_RNG
// Mersenne-Twister RNG.
// @note:	Isn't 100% thread safe. It is possible two threads will
//			get the same value.
namespace Random
{
	void Init(unsigned long seed);
	// Full range.
	unsigned long NextInt();
	// [0, 1) range.
	float NextFloat();
}
#endif

// Simple, very fast LCG random number generator.
// Thread safe, because it can be instantiated.
// Taken from Intel's TBB (http://www.threadingbuildingblocks.org/)
// Random int generation takes about 25 ticks on C2D.
class FastRandom
{
public:
	explicit FastRandom(unsigned long seed);

	// Full range.
	unsigned long NextInt();
	// [0, 1)
	float NextFloat();

private:
	unsigned long m_x;
	unsigned long m_a;
};

// KISS random generator.
// Taken from "Good Practice in (Pseudo) Number Generation for
// Bioinformatics Applications" by David Jones.
// Thread safe, because it can be instantiated.
// About 30 ticks on C2D.
class KISSRandom
{
public:
	explicit KISSRandom(unsigned long x = 123456789,
		unsigned long y = 362436069, 
		unsigned long z = 21288629, 
		unsigned long w = 14921776);

	void Seed(unsigned long x);

	// Full range.
	unsigned long NextInt();
	// [0, 1)
	float NextFloat();

private:
	unsigned long m_a;
	unsigned long m_x;
	unsigned long m_y;
	unsigned long m_z;
	unsigned long m_w;
};

}

#endif // 
