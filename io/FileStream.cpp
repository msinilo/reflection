#include "io/FileStream.h"
#if !RDE_IO_STANDALONE
#	include "core/RdeAssert.h"
#endif

namespace rde
{
FileStream::FileStream()
:	m_handle(0)
{
	/**/
}
FileStream::~FileStream()
{
	if (IsOpen())
		Close();
}
bool FileStream::Open(const char* fileName, unsigned long accessModeFlags)
{
	RDE_ASSERT(!IsOpen());
	if (IsOpen())
		Close();
	m_handle = iosys::OpenFile(fileName, accessModeFlags);
	if (IsOpen())
		m_accessMode = accessModeFlags;
	return IsOpen();
}
long FileStream::Read(void* data, long bytes)
{
	RDE_ASSERT(IsOpen());
	RDE_ASSERT(GetAccessModeFlags() & iosys::AccessMode::READ);
	if (IsOpen())
		return iosys::Read(m_handle, data, bytes);
	else
		return 0;
}
void FileStream::Write(const void* data, long bytes)
{
	RDE_ASSERT(IsOpen());
	RDE_ASSERT(GetAccessModeFlags() & iosys::AccessMode::WRITE);
	if (IsOpen())
		iosys::Write(m_handle, data, bytes);
}
void FileStream::Flush()
{
	if (IsOpen())
		iosys::FlushFile(m_handle);
}
void FileStream::Seek(iosys::SeekMode::Enum mode, long offset)
{
	if (IsOpen())
		iosys::SeekFile(m_handle, mode, offset);
}
void FileStream::Close()
{
	RDE_ASSERT(IsOpen());
	iosys::CloseFile(m_handle);
	m_handle = 0;
}
long FileStream::GetSize() const
{
	return IsOpen() ? iosys::GetFileSize(m_handle) : 0;
}
long FileStream::GetPosition() const
{
	return IsOpen() ? iosys::GetFilePosition(m_handle) : 0;
}
}