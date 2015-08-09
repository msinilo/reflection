#ifndef IO_IOSYS_H
#define IO_IOSYS_H
// System related low-level IO functions.

#define RDE_IO_STANDALONE	0

#if RDE_IO_STANDALONE
#	include <cassert>
#	include <cstring>
#	define RDE_ASSERT	assert
#else
#	include "core/Config.h"
#endif

namespace rde
{
#if RDE_IO_STANDALONE
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned long	uint32_t;

namespace Sys
{
void MemCpy(void* to, const void* from, size_t bytes)
{
	memcpy(to, from, bytes);
}
} // Sys
#endif

namespace iosys
{
	typedef void*	FileHandle;
	namespace AccessMode
	{
		enum Enum
		{
			READ		= 0x1,
			WRITE		= 0x2,
			READWRITE	= READ | WRITE
		};
	}
	namespace SeekMode
	{
		enum Enum
		{
			BEGIN,
			CURRENT,
			END
		};
	}

	const FileHandle	INVALID_FILE_HANDLE	= 0;

	FileHandle OpenFile(const char* path, unsigned long accessModeFlags);
	void CloseFile(FileHandle f);
	long Read(FileHandle f, void* data, long bytes);
	void Write(FileHandle f, const void* data, long bytes);
	long GetFileSize(FileHandle f);
	void SeekFile(FileHandle f, SeekMode::Enum mode, long offset);
	long GetFilePosition(FileHandle f);
	void FlushFile(FileHandle f);

	bool Exists(const char* path);
} // iosys
}

#endif // IO_IOSYS_H
