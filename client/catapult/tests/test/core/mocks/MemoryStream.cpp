#include "MemoryStream.h"
#include "catapult/exceptions.h"

namespace catapult { namespace mocks {

	MemoryStream::MemoryStream(const std::string& name, std::vector<uint8_t>& buffer)
			: m_name(name)
			, m_buffer(buffer)
			, m_readPosition(0)
			, m_flushCount(0) {
		m_buffer.reserve(1024);
	}

	void MemoryStream::write(const utils::RawBuffer& buffer) {
		m_buffer.insert(m_buffer.end(), buffer.pData, buffer.pData + buffer.Size);
	}

	void MemoryStream::read(const utils::MutableRawBuffer& buffer) {
		if (m_readPosition + buffer.Size > m_buffer.size())
			CATAPULT_THROW_FILE_IO_ERROR("MemoryStream read error");

		std::memcpy(buffer.pData, m_buffer.data() + m_readPosition, buffer.Size);
		m_readPosition += buffer.Size;
	}

	void MemoryStream::flush() {
		++m_flushCount;
	}

	size_t MemoryStream::numFlushes() const {
		return m_flushCount;
	}

	size_t MemoryStream::position() const {
		return m_readPosition;
	}
}}
