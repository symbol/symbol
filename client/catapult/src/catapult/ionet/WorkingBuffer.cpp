#include "WorkingBuffer.h"

namespace catapult { namespace ionet {

	WorkingBuffer::WorkingBuffer(const PacketSocketOptions& options)
			: m_options(options)
			, m_numDataSizeSamples(0)
			, m_maxDataSize(0) {
		m_data.reserve(m_options.WorkingBufferSize);
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

		CATAPULT_LOG(debug) << "reclaiming memory, decreasing buffer capacity from " << m_data.capacity() << " to " << maxDataSize;

		ByteBuffer dataCopy;
		dataCopy.reserve(maxDataSize);
		dataCopy.resize(m_data.size());
		std::memcpy(dataCopy.data(), m_data.data(), m_data.size());
		std::swap(m_data, dataCopy);
	}
}}
