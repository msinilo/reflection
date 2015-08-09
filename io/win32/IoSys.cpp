#include "io/IoSys.h"

#if !RDE_IO_STANDALONE
//#	include "core/Console.h"
#	include "core/RdeAssert.h"
//#	include "core/win32/Windows.h"
#endif
#	define WIN32_LEAN_AND_MEAN	1
#	include <windows.h>
//#endif

namespace rde
{
iosys::FileHandle iosys::OpenFile(const char* path, unsigned long accessModeFlags)
{
	DWORD sysAccess(0);
	DWORD shareMode(0);
	DWORD creationDisposition(0);
	if (accessModeFlags & AccessMode::READ)
	{
		sysAccess |= GENERIC_READ;
		shareMode |= FILE_SHARE_READ | FILE_SHARE_WRITE;
		creationDisposition = OPEN_EXISTING;
	}
	if (accessModeFlags & AccessMode::WRITE)
	{
		sysAccess |= GENERIC_WRITE;
		shareMode |= FILE_SHARE_WRITE;
		creationDisposition = CREATE_ALWAYS;
	}
	if ((accessModeFlags & AccessMode::READWRITE) == AccessMode::READWRITE)
		creationDisposition = OPEN_ALWAYS;
	HANDLE hFile = ::CreateFile(path, sysAccess, shareMode, 0, 
		creationDisposition, FILE_FLAG_RANDOM_ACCESS, 0);
	// Translate to our convention.
	if (hFile == INVALID_HANDLE_VALUE)
		hFile = INVALID_FILE_HANDLE;
	return hFile;
}
void iosys::CloseFile(FileHandle f)
{
	RDE_ASSERT(f != INVALID_FILE_HANDLE);
	::CloseHandle(f);
}
long iosys::Read(FileHandle f, void* data, long bytes)
{
	RDE_ASSERT(f != INVALID_FILE_HANDLE);
	RDE_ASSERT(data != 0);
	RDE_ASSERT(bytes > 0);
	DWORD bytesRead(0);
	const BOOL success = ::ReadFile(f, data, bytes, &bytesRead, 0);
	RDE_ASSERT(success);
#if !RDE_IO_STANDALONE
	//if (!success)
	//	Console::Errorf("Error while reading %d bytes from file %p", bytes, f);
#endif
	return bytesRead;
}
void iosys::Write(FileHandle f, const void* data, long bytes)
{
	RDE_ASSERT(f != INVALID_FILE_HANDLE);
	RDE_ASSERT(data != 0);
	RDE_ASSERT(bytes > 0);
	DWORD bytesWritten(0);
	const BOOL success = ::WriteFile(f, data, bytes, &bytesWritten, 0);
	RDE_ASSERT(success);
#if !RDE_IO_STANDALONE
	//if (!success || (long)bytesWritten != bytes)
	//	Console::Errorf("Error while writing %d bytes to file %p", bytes, f);
#endif
}
long iosys::GetFileSize(FileHandle f)
{
	RDE_ASSERT(f != INVALID_FILE_HANDLE);
	return ::GetFileSize(f, 0);
}
void iosys::SeekFile(FileHandle f, SeekMode::Enum mode, long offset)
{
	RDE_ASSERT(f != INVALID_FILE_HANDLE);
	RDE_ASSERT(mode <= SeekMode::END);
	const DWORD winSeekModes[] =
	{
		FILE_BEGIN, FILE_CURRENT, FILE_END
	};
	::SetFilePointer(f, offset, 0, winSeekModes[mode]);
}
long iosys::GetFilePosition(FileHandle f)
{
	RDE_ASSERT(f != INVALID_FILE_HANDLE);
	return ::SetFilePointer(f, 0, NULL, FILE_CURRENT);
}
void iosys::FlushFile(FileHandle f)
{
	RDE_ASSERT(f != INVALID_FILE_HANDLE);
	::FlushFileBuffers(f);
}

bool iosys::Exists(const char* path)
{
	return ::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
}

} // rde