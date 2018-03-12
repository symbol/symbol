#include "PacketExtractor.h"
#include "catapult/utils/Logging.h"
#include <cstring>

namespace catapult { namespace ionet {

	namespace {
		constexpr bool IsSizeValid(size_t packetSize, size_t maxPacketDataSize) {
			return packetSize >= sizeof(PacketHeader) && (packetSize - sizeof(PacketHeader)) <= maxPacketDataSize;
		}
	}

	PacketExtractor::PacketExtractor(ByteBuffer& data, size_t maxPacketDataSize)
			: m_data(data)
			, m_maxPacketDataSize(maxPacketDataSize)
			, m_consumedBytes(0)
	{}

	PacketExtractResult PacketExtractor::tryExtractNextPacket(const Packet*& pExtractedPacket) {
		pExtractedPacket = nullptr;
		auto remainingDataSize = m_data.size() - m_consumedBytes;
		if (remainingDataSize < sizeof(uint32_t))
			return PacketExtractResult::Insufficient_Data;

		const Packet* pPacket = reinterpret_cast<const Packet*>(&m_data[m_consumedBytes]);
		if (!IsSizeValid(pPacket->Size, m_maxPacketDataSize)) {
			CATAPULT_LOG(warning)
					<< "unable to extract packet with size " << pPacket->Size
					<< " (" << m_data.size() << " bytes, " << remainingDataSize << " remaining, " << m_consumedBytes << " consumed)";
			return PacketExtractResult::Packet_Error;
		}

		if (pPacket->Size > remainingDataSize)
			return PacketExtractResult::Insufficient_Data;

		pExtractedPacket = pPacket;
		m_consumedBytes += pPacket->Size;
		return PacketExtractResult::Success;
	}

	void PacketExtractor::consume() {
		if (0 == m_consumedBytes)
			return;

		auto remainingDataSize = m_data.size() - m_consumedBytes;
		if (0 != remainingDataSize)
			std::memmove(m_data.data(), &m_data[m_consumedBytes], remainingDataSize);

		m_data.resize(remainingDataSize);
		m_consumedBytes = 0;
	}
}}
