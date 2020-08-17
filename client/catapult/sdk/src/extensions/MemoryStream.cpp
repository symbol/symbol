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

#include "MemoryStream.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/exceptions.h"
#include <sstream>

namespace catapult { namespace extensions {

	MemoryStream::MemoryStream(std::vector<uint8_t>& buffer)
			: m_buffer(buffer)
			, m_position(0)
	{}

	void MemoryStream::write(const RawBuffer& buffer) {
		m_buffer.resize(std::max<size_t>(m_buffer.size(), m_position + buffer.Size));
		utils::memcpy_cond(&m_buffer[m_position], buffer.pData, buffer.Size);
		m_position += buffer.Size;
	}

	void MemoryStream::flush()
	{}

	bool MemoryStream::eof() const {
		return m_position == m_buffer.size();
	}

	void MemoryStream::read(const MutableRawBuffer& buffer) {
		if (buffer.Size + m_position > m_buffer.size()) {
			std::ostringstream out;
			out
					<< "MemoryStream invalid read (read-size = " << buffer.Size
					<< ", stream-position = " << m_position
					<< ", stream-size = " << m_buffer.size() << ")";
			CATAPULT_THROW_FILE_IO_ERROR(out.str().c_str());
		}

		utils::memcpy_cond(buffer.pData, m_buffer.data() + m_position, buffer.Size);
		m_position += buffer.Size;
	}

	void MemoryStream::seek(uint64_t position) {
		if (position > m_buffer.size()) {
			std::ostringstream out;
			out << "MemoryStream invalid seek (seek-position = " << position << ", stream-size = " << m_buffer.size() << ")";
			CATAPULT_THROW_FILE_IO_ERROR(out.str().c_str());
		}

		m_position = position;
	}

	uint64_t MemoryStream::position() const {
		return m_position;
	}
}}
