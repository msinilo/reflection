#ifndef IO_FILE_STREAM_H
#define IO_FILE_STREAM_H

#include "io/Stream.h"

namespace rde
{
class FileStream : public Stream
{
public:
	FileStream();
	virtual ~FileStream();

	bool Open(const char* fileName, unsigned long accessModeFlags);

	virtual long Read(void* data, long bytes);
	virtual void Write(const void* data, long bytes);
	virtual void Flush();
	virtual void Seek(iosys::SeekMode::Enum mode, long offset);
	virtual void Close();

	virtual bool IsOpen() const	{ return m_handle != 0; }
	virtual long GetSize() const;
	virtual long GetPosition() const;

private:
	iosys::FileHandle	m_handle;
};

} // rde

#endif // IO_FILE_STREAM_H
