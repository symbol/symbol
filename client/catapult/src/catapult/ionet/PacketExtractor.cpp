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

#include "PacketExtractor.h"
#include "catapult/utils/Logging.h"
#include <cstring>

namespace catapult { namespace ionet {

	PacketExtractor::PacketExtractor(ByteBuffer& data, size_t maxPacketDataSize)
			: m_data(data)
			, m_maxPacketDataSize(maxPacketDataSize)
			, m_consumedBytes(0)
	{}

	PacketExtractResult PacketExtractor::tryExtractNextPacket(const Packet*& pExtractedPacket) {
		pExtractedPacket = nullptr;
		auto remainingDataSize = m_data.size() - m_consumedBytes;
		if (remainingDataSize < sizeof(PacketHeader))
			return PacketExtractResult::Insufficient_Data;

		const auto& packet = reinterpret_cast<const Packet&>(m_data[m_consumedBytes]);
		if (!IsPacketDataSizeValid(packet, m_maxPacketDataSize)) {
			CATAPULT_LOG(warning)
					<< "unable to extract " << packet
					<< " (" << m_data.size() << " bytes, " << remainingDataSize << " remaining, " << m_consumedBytes << " consumed)";
			return PacketExtractResult::Packet_Error;
		}

		if (packet.Size > remainingDataSize)
			return PacketExtractResult::Insufficient_Data;

		pExtractedPacket = &packet;
		m_consumedBytes += packet.Size;
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
