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
#include "catapult/exceptions.h"

namespace catapult { namespace mocks {

	MockMemoryStream::MockMemoryStream(const std::string& name, std::vector<uint8_t>& buffer)
			: m_name(name)
			, m_buffer(buffer)
			, m_readPosition(0)
			, m_flushCount(0) {
		m_buffer.reserve(1024);
	}

	void MockMemoryStream::write(const RawBuffer& buffer) {
		m_buffer.insert(m_buffer.end(), buffer.pData, buffer.pData + buffer.Size);
	}

	void MockMemoryStream::read(const MutableRawBuffer& buffer) {
		if (m_readPosition + buffer.Size > m_buffer.size())
			CATAPULT_THROW_FILE_IO_ERROR("MockMemoryStream read error");

		std::memcpy(buffer.pData, m_buffer.data() + m_readPosition, buffer.Size);
		m_readPosition += buffer.Size;
	}

	void MockMemoryStream::flush() {
		++m_flushCount;
	}

	size_t MockMemoryStream::numFlushes() const {
		return m_flushCount;
	}

	size_t MockMemoryStream::position() const {
		return m_readPosition;
	}
}}
