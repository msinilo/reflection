#include "io/StreamReader.h"
#include "io/Stream.h"

namespace rde
{
StreamReader::StreamReader(Stream* stream)
:	m_stream(stream)
{
	/**/
}
StreamReader::~StreamReader()
{
	/**/
}

long StreamReader::Read(void* data, long bytes)
{
	return m_stream->Read(data, bytes);
}
int32_t StreamReader::ReadInt32()
{
	int32_t i;
	m_stream->Read(&i, sizeof(i));
	return i;
}
int16_t StreamReader::ReadInt16()
{
	int16_t i;
	m_stream->Read(&i, sizeof(i));
	return i;
}

void StreamReader::ReadASCIIZ(char* buffer, long /*bufferSize*/)
{
	const int16_t strLen = ReadInt16();
	if (strLen)
		m_stream->Read(buffer, strLen);
	buffer[strLen] = '\0';
}

void StreamReader::Flush()
{
	m_stream->Flush();
}
void StreamReader::Close()
{
	m_stream->Close();
}

} // rde

