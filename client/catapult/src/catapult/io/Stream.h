#pragma once
#include "catapult/types.h"

namespace catapult { namespace io {

	/// Reader interface.
	class InputStream {
	public:
		virtual ~InputStream() {}

		/// Reads data from this stream into \a buffer.
		/// \throws catapult_file_io_error if requested amount of data could not be read.
		virtual void read(const MutableRawBuffer& buffer) = 0;
	};

	/// Writer interface.
	class OutputStream {
	public:
		virtual ~OutputStream() {}

		/// Writes data pointed to by \a buffer to this stream.
		/// \throws catapult_file_io_error if proper amount of data could not be written.
		virtual void write(const RawBuffer& buffer) = 0;

		/// Commits all pending data.
		/// \throws catapult_file_io_error if flush failed.
		virtual void flush() = 0;
	};
}}
