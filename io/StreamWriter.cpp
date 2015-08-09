#include "io/StreamWriter.h"
#include "io/Stream.h"
#if !RDE_IO_STANDALONE
#	include "core/RdeAssert.h"
#endif

namespace rde
{
StreamWriter::StreamWriter(Stream* stream)
:	m_stream(stream)
{
	/**/
}
StreamWriter::~StreamWriter()
{
	/**/
}

void StreamWriter::Write(const void* data, long bytes)
{
	m_stream->Write(data, bytes);
}
void StreamWriter::WriteInt32(int32_t i)
{
	m_stream->Write(&i, sizeof(i));
}
void StreamWriter::WriteInt16(int16_t i)
{
	m_stream->Write(&i, sizeof(i));
}

void StreamWriter::WriteASCIIZ(const char* str)
{
	const long len = (long)strlen(str);
	RDE_ASSERT(len < 32768 && "String too long, cannot write to stream");
	WriteInt16(rde::uint16_t(len));
	if (len > 0)
	{
		Write(str, len);
	}
}

void StreamWriter::Flush()
{
	m_stream->Flush();
}
void StreamWriter::Close()
{
	m_stream->Close();
}

}