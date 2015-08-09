#include <cstring>

namespace rde
{
__forceinline void Sys::MemCpy(void* to, const void* from, size_t bytes)
{
	memcpy(to, from, bytes);
}
__forceinline void Sys::MemMove(void* to, const void* from, size_t bytes)
{
	memmove(to, from, bytes);
}
__forceinline void Sys::MemSet(void* buf, uint8_t value, size_t bytes)
{
	memset(buf, value, bytes);
}

namespace Sys
{
	void GetErrorString(char* outString, int maxOutStringLen, uint32_t errorOverride = 0);
}

}
