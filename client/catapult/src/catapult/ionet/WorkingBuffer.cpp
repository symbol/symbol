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

#include "WorkingBuffer.h"

namespace catapult { namespace ionet {

	WorkingBuffer::WorkingBuffer(const PacketSocketOptions& options)
			: m_options(options)
			, m_numDataSizeSamples(0)
			, m_maxDataSize(0) {
		m_data.reserve(m_options.WorkingBufferSize);
	}

	void WorkingBuffer::append(uint8_t byte) {
		m_data.push_back(byte);
	}

	AppendContext WorkingBuffer::prepareAppend() {
		AppendContext appendContext(m_data, m_options.WorkingBufferSize);
		checkMemoryUsage();
		return appendContext;
	}

	PacketExtractor WorkingBuffer::preparePacketExtractor() {
		return PacketExtractor(m_data, m_options.MaxPacketDataSize);
	}

	void WorkingBuffer::checkMemoryUsage() {
		// ignore if memory reclamation is disabled
		if (0 == m_options.WorkingBufferSensitivity)
			return;

		// record a sample but only check at intervals to minimize impact
		m_maxDataSize = std::max(m_maxDataSize, m_data.size());
		if (++m_numDataSizeSamples != m_options.WorkingBufferSensitivity)
			return;

		// ignore if savings is less than WorkingBufferSize
		auto maxDataSize = m_maxDataSize;
		m_numDataSizeSamples = 0;
		m_maxDataSize = 0;
		if (m_data.capacity() - maxDataSize < m_options.WorkingBufferSize)
			return;

		CATAPULT_LOG(trace) << "reclaiming memory, decreasing buffer capacity from " << m_data.capacity() << " to " << maxDataSize;

		ByteBuffer dataCopy;
		dataCopy.reserve(maxDataSize);
		dataCopy.resize(m_data.size());
		std::memcpy(dataCopy.data(), m_data.data(), m_data.size());
		std::swap(m_data, dataCopy);
	}
}}
