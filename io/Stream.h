#ifndef IO_STREAM_H
#define IO_STREAM_H

#include "io/IoSys.h"

namespace rde
{
class Stream
{
	Stream(const Stream&);
	Stream& operator=(const Stream&);

public:
	virtual ~Stream() {}

	// Returns number of bytes really read.
	virtual long Read(void* data, long bytes) = 0;
	virtual void Write(const void* data, long bytes) = 0;
	virtual void Flush() {}
	virtual void Seek(iosys::SeekMode::Enum mode, long offset) = 0;
	virtual void Close() {}

	virtual long GetSize() const = 0;
	virtual long GetPosition() const = 0;
	virtual bool IsOpen() const = 0;

	unsigned long GetAccessModeFlags() const { return m_accessMode; }

protected:
	Stream() {}

	unsigned long	m_accessMode;
};

} // rde

#endif // IO_STREAM_H
