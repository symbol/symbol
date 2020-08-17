/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "RawFile.h"
#include "Stream.h"
#include <vector>

namespace catapult { namespace io {

	/// Default stream buffer size.
	constexpr size_t Default_Stream_Buffer_Size = 4096;

	// region BufferedOutputFileStream

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

	// endregion

	// region BufferedInputFileStream

	/// Provides a buffered input stream around raw file.
	class BufferedInputFileStream final : public InputStream {
	public:
		/// Creates a buffered input stream around \a rawFile with an optional internal buffer size (\a bufferSize).
		BufferedInputFileStream(RawFile&& rawFile, size_t bufferSize = Default_Stream_Buffer_Size);

	public:
		bool eof() const override;
		void read(const MutableRawBuffer& buffer) override;

	private:
		RawFile m_rawFile;
		std::vector<uint8_t> m_buffer;
		size_t m_bufferPosition; // position of next read
		// during reading m_numBytesInBuffer mostly will be m_buffer.size() and might be smaller near the end of file
		size_t m_numBytesInBuffer;
	};

	// endregion
}}
