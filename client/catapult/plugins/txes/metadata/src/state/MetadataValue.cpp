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

#include "MetadataValue.h"

namespace catapult { namespace state {

	bool MetadataValue::empty() const {
		return m_buffer.empty();
	}

	size_t MetadataValue::size() const {
		return m_buffer.size();
	}

	const uint8_t* MetadataValue::data() const {
		return m_buffer.data();
	}

	bool MetadataValue::canTrim(const RawBuffer& buffer, size_t count) const {
		// if parameters are invalid, return false to prevent crash
		if (m_buffer.size() != buffer.Size || count > m_buffer.size())
			return false;

		for (auto i = buffer.Size - count; i < buffer.Size; ++i) {
			if (m_buffer[i] != buffer.pData[i])
				return false;
		}

		return true;
	}

	void MetadataValue::update(const RawBuffer& buffer) {
		m_buffer.resize(buffer.Size);
		for (auto i = 0u; i < buffer.Size; ++i)
			m_buffer[i] ^= buffer.pData[i];
	}
}}
