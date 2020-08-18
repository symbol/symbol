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
#include "sdk/src/extensions/MemoryStream.h"

namespace catapult { namespace mocks {

	// region MockMemoryStream

	/// Memory-based implementation of input and output stream.
	class MockMemoryStream final : public extensions::MemoryStream {
	public:
		/// Creates a memory stream around \a buffer.
		explicit MockMemoryStream(std::vector<uint8_t>& buffer);

	public:
		void flush() override;

	public:
		/// Gets the number of times flush was invoked.
		size_t numFlushes() const;

	private:
		size_t m_flushCount;
	};

	// endregion

	// region MockSeekableMemoryStream

	/// Memory-based implementation of seekable input and output stream.
	class MockSeekableMemoryStream : public extensions::MemoryStream {
	public:
		/// Creates an empty memory stream.
		MockSeekableMemoryStream();

	public:
		/// Copies the contents of this stream into \a dest.
		void copyTo(MockSeekableMemoryStream& dest) const;

	private:
		std::vector<uint8_t> m_buffer;
	};

	// endregion
}}
