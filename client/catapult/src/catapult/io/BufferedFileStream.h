#pragma once
#include "RawFile.h"
#include "Stream.h"
#include <vector>

namespace catapult { namespace io {

	/// The default stream buffer size.
	constexpr size_t Default_Stream_Buffer_Size = 4096;

	/// Provides a buffered output stream around raw file.
	class BufferedOutputFileStream final : public OutputStream {
	public:
		/// Creates a buffered output stream around \a rawFile with an optional internal buffer size (\a bufferSize).
		BufferedOutputFileStream(RawFile&& rawFile, size_t bufferSize = Default_Stream_Buffer_Size);

	public:
		void write(const RawBuffer& buffer) override;

		void flush() override;

	private:
		RawFile m_rawFile;
		std::vector<uint8_t> m_buffer;
		size_t m_bufferPosition; // position of next write
	};

	/// Provides a buffered input stream around raw file.
	class BufferedInputFileStream final : public InputStream {
	public:
		/// Creates a buffered input stream around \a rawFile with an optional internal buffer size (\a bufferSize).
		BufferedInputFileStream(RawFile&& rawFile, size_t bufferSize = Default_Stream_Buffer_Size);

	public:
		void read(const MutableRawBuffer& buffer) override;

	private:
		RawFile m_rawFile;
		std::vector<uint8_t> m_buffer;
		size_t m_bufferPosition; // position of next read
		// during reading m_numBytesInBuffer mostly will be m_buffer.size() and might be smaller near the end of file
		size_t m_numBytesInBuffer;
	};
}}
