#ifndef IO_STREAM_WRITER_H
#define IO_STREAM_WRITER_H

#include "io/IoSys.h"

namespace rde
{
class Stream;

// Stream writer.
// Default implementation simply forwards calls to underlying stream.
class StreamWriter
{
public:
	explicit StreamWriter(Stream* stream);
	virtual ~StreamWriter();

	virtual void Write(const void* data, long bytes);
	virtual void WriteInt32(int32_t i);
	virtual void WriteInt16(int16_t i);
	virtual void WriteASCIIZ(const char* str);
	virtual void Flush();
	virtual void Close();

	Stream* GetStream() const	{ return m_stream; }
protected:
	Stream*	m_stream;
};
} // rde

#endif // IO_STREAM_WRITER_H
