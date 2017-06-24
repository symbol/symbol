#include "WorkingBuffer.h"

namespace catapult { namespace ionet {

	WorkingBuffer::WorkingBuffer(const PacketSocketOptions& options) : m_options(options) {
		m_data.reserve(m_options.WorkingBufferSize);
	}

	AppendContext WorkingBuffer::prepareAppend() {
		return AppendContext(m_data, m_options.WorkingBufferSize);
	}

	PacketExtractor WorkingBuffer::preparePacketExtractor() {
		return PacketExtractor(m_data, m_options.MaxPacketDataSize);
	}
}}
