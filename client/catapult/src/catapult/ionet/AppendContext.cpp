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

#include "AppendContext.h"
#include "catapult/exceptions.h"

namespace catapult { namespace ionet {

	AppendContext::AppendContext(ByteBuffer& data, size_t size)
			: m_data(data)
			, m_originalSize(m_data.size())
			, m_isCommitted(false) {
		// only guarantee that at least half of size can be appended (safe because buffer() uses correct size)
		// in cases where boost sends only 8-byte packet headers, this will use less memory than guaranteeing full size
		if (m_data.capacity() - m_data.size() < size / 2)
			m_data.resize(m_originalSize + size);
		else
			m_data.resize(std::min(m_originalSize + size, m_data.capacity()));

		m_appendSize = m_data.size() - m_originalSize;
	}

	AppendContext::AppendContext(AppendContext&& rhs)
			: m_data(rhs.m_data)
			, m_appendSize(rhs.m_appendSize)
			, m_originalSize(rhs.m_originalSize)
			, m_isCommitted(rhs.m_isCommitted) {
		// mark rhs as committed to prevent it from abandoning changes
		rhs.m_isCommitted = true;
	}

	AppendContext::~AppendContext() {
		if (!m_isCommitted)
			m_data.resize(m_originalSize);
	}

	boost::asio::mutable_buffers_1 AppendContext::buffer() {
		assertNotCommitted();
		return boost::asio::buffer(&m_data[m_originalSize], m_appendSize);
	}

	void AppendContext::commit(size_t size) {
		assertNotCommitted();
		if (size > m_appendSize)
			CATAPULT_THROW_RUNTIME_ERROR_2("cannot commit more than reserved append size", size, m_appendSize);

		m_data.resize(m_originalSize + size);
		m_isCommitted = true;
	}

	void AppendContext::assertNotCommitted() const {
		if (m_isCommitted)
			CATAPULT_THROW_RUNTIME_ERROR("append context was already committed");
	}
}}
