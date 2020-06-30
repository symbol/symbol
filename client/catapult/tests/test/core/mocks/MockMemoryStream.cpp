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

#include "MockMemoryStream.h"

namespace catapult { namespace mocks {

	// region MockMemoryStream

	MockMemoryStream::MockMemoryStream(std::vector<uint8_t>& buffer)
			: MemoryStream(buffer)
			, m_flushCount(0)
	{}

	void MockMemoryStream::flush() {
		++m_flushCount;
	}

	size_t MockMemoryStream::numFlushes() const {
		return m_flushCount;
	}

	// endregion

	// region MockSeekableMemoryStream

	MockSeekableMemoryStream::MockSeekableMemoryStream() : m_position(0)
	{}

	void MockSeekableMemoryStream::write(const RawBuffer& buffer) {
		m_buffer.resize(std::max<size_t>(m_buffer.size(), m_position + buffer.Size));
		std::memcpy(&m_buffer[m_position], buffer.pData, buffer.Size);
		m_position += buffer.Size;
	}

	void MockSeekableMemoryStream::flush()
	{}

	void MockSeekableMemoryStream::seek(uint64_t position) {
		m_position = position;
	}

	uint64_t MockSeekableMemoryStream::position() const {
		return m_position;
	}

	bool MockSeekableMemoryStream::eof() const {
		return m_position == m_buffer.size();
	}

	void MockSeekableMemoryStream::read(const MutableRawBuffer& buffer) {
		if (m_position + buffer.Size > m_buffer.size())
			CATAPULT_THROW_RUNTIME_ERROR("invalid read()");

		std::memcpy(buffer.pData, &m_buffer[m_position], buffer.Size);
		m_position += buffer.Size;
	}

	// endregion
}}
