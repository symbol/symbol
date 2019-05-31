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

namespace catapult { namespace extensions {

	MemoryStream::MemoryStream(std::vector<uint8_t>& buffer)
			: io::BufferInputStreamAdapter<std::vector<uint8_t>>(buffer)
			, m_buffer(buffer) {
		m_buffer.reserve(1024);
	}

	void MemoryStream::write(const RawBuffer& buffer) {
		m_buffer.insert(m_buffer.end(), buffer.pData, buffer.pData + buffer.Size);
	}

	void MemoryStream::flush()
	{}
}}
