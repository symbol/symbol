/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "catapult/plugins.h"
#include "catapult/types.h"

namespace catapult { namespace io {

	/// Reader interface.
	class PLUGIN_API_DEPENDENCY InputStream {
	public:
		virtual ~InputStream() = default;

	public:
		/// Returns \c true if no data is left in the stream.
		virtual bool eof() const = 0;

		/// Reads data from this stream into \a buffer.
		/// \throws catapult_file_io_error if requested amount of data could not be read.
		virtual void read(const MutableRawBuffer& buffer) = 0;
	};

	/// Writer interface.
	class PLUGIN_API_DEPENDENCY OutputStream {
	public:
		virtual ~OutputStream() = default;

	public:
		/// Writes data pointed to by \a buffer to this stream.
		/// \throws catapult_file_io_error if proper amount of data could not be written.
		virtual void write(const RawBuffer& buffer) = 0;

		/// Commits all pending data.
		/// \throws catapult_file_io_error if flush failed.
		virtual void flush() = 0;
	};
}}
