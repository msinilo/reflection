#ifndef CORE_CRC32_H
#define CORE_CRC32_H

#include "core/Config.h"

namespace rde
{
// Class representing CRC32 checksum value.
class CRC32
{
public:
	CRC32();
	explicit CRC32(const char* asciiz);

	void operator=(const char* asciiz);

	void Add8(uint8_t);
	void Add16(uint16_t);
	void Add32(uint32_t);

	void AddArray(const rde::uint8_t* bytes, long numBytes);

	void Reset();
	uint32_t GetValue() const	{ return m_value; }

	static uint32_t GetValue(const char* asciiz);

private:
	uint32_t	m_value;
};

inline bool operator==(const CRC32& lhs, const CRC32& rhs)
{
	return lhs.GetValue() == rhs.GetValue();
}
inline bool operator!=(const CRC32& lhs, const CRC32& rhs)
{
	return !(lhs == rhs);
}
inline bool operator<(const CRC32& lhs, const CRC32& rhs)
{
	return lhs.GetValue() < rhs.GetValue();
}
}

#endif // CORE_CRC32_H
