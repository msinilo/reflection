#include "core/Random.h"
#include "core/Config.h"
#if RDE_NEEDS_MERSENNE_TWISTER_RNG
#	include "core/mt/SFMT.h"
#endif

namespace
{
static const unsigned long s_primes[] = 
{
    0x9e3779b1, 0xffe6cc59, 0x2109f6dd, 0x43977ab5,
    0xba5703f5, 0xb495a877, 0xe1626741, 0x79695e6b,
    0xbc98c09f, 0xd5bee2b3, 0x287488f9, 0x3af18231,
    0x9677cd4d, 0xbe3a6929, 0xadc6a877, 0xdcf0674b,
    0xbe4d6fe9, 0x5f15e201, 0x99afc3fd, 0xf3f16801,
    0xe222cfff, 0x24ba5fdb, 0x0620452d, 0x79f149e3,
    0xc8b93f49, 0x972702cd, 0xb07dd827, 0x6c97d5ed,
    0x085a3d61, 0x46eb5ea7, 0x3d9910ed, 0x2e687b5b,
    0x29609227, 0x6eb081f1, 0x0954c4e1, 0x9d114db9,
    0x542acfa9, 0xb3e6bd7b, 0x0742d917, 0xe9f3ffa7,
    0x54581edb, 0xf2480f45, 0x0bb9288f, 0xef1affc7,
    0x85fa0ca7, 0x3ccc14db, 0xe6baf34b, 0x343377f7,
    0x5ca19031, 0xe6d9293b, 0xf0a9f391, 0x5d2e980b,
    0xfc411073, 0xc3749363, 0xb892d829, 0x3549366b,
    0x629750ad, 0xb98294e5, 0x892d9483, 0xc235baf3,
    0x3d2402a3, 0x6bdef3c9, 0xbec333cd, 0x40c9520f
}; 
} // <anonymous> namespace

namespace rde
{
#if RDE_NEEDS_MERSENNE_TWISTER_RNG
void Random::Init(unsigned long seed)
{
	init_gen_rand(seed);
}

unsigned long Random::NextInt()
{
	return gen_rand32();
}

float Random::NextFloat()
{
	return (float)genrand_real2();
}
#endif

FastRandom::FastRandom(unsigned long seed)
{
	m_x = seed;
	m_a = s_primes[seed % RDE_ARRAY_COUNT(s_primes)];
}

unsigned long FastRandom::NextInt() 
{
	unsigned long ret = m_x;
	m_x = m_x * m_a + 1;
	return ret;
}

float FastRandom::NextFloat()
{
	const unsigned long i = NextInt() & 0x7FFFFFFF;
	static const float kInv = 1.f / float(0x80000000);
	return float(i) * kInv;
}

KISSRandom::KISSRandom(unsigned long x, unsigned long y,
					   unsigned long z, unsigned long w)
:	m_x(x), m_y(y), m_z(z), m_w(w), m_a(0)
{
	/**/
}

void KISSRandom::Seed(unsigned long x)
{
	m_x = x;
}

unsigned long KISSRandom::NextInt()
{
	m_x += 545925293;
	m_y ^= (m_y << 13);
	m_y ^= (m_y >> 17);
	m_y ^= (m_y << 5);
	const unsigned long t = m_z + m_w + m_a;
	m_z = m_w;
	m_a = t >> 31;
	m_w = t & 0x7FFFFFFF;
	return m_x + m_y + m_w;
}

float KISSRandom::NextFloat()
{
	const unsigned long i = NextInt() & 0x7FFFFFFF;
	static const float kInv = 1.f / float(0x80000000);
	return float(i) * kInv;
}

} // rde
