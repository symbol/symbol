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

#include "PacketPayload.h"

namespace catapult { namespace ionet {

	PacketPayload::PacketPayload() {
		m_header.Size = 0u;
		m_header.Type = PacketType::Undefined;
	}

	PacketPayload::PacketPayload(PacketType type) {
		m_header.Size = sizeof(PacketHeader);
		m_header.Type = type;
	}

	PacketPayload::PacketPayload(const std::shared_ptr<const Packet>& pPacket) {
		if (pPacket->Size < sizeof(PacketHeader))
			CATAPULT_THROW_INVALID_ARGUMENT("cannot create payload around packet with invalid size");

		m_header = *pPacket;
		if (pPacket->Size == sizeof(PacketHeader))
			return;

		m_entities.push_back(pPacket);
		m_buffers.push_back({ pPacket->Data(), m_header.Size - sizeof(PacketHeader) });
	}

	bool PacketPayload::unset() const {
		return 0u == m_header.Size;
	}

	const PacketHeader& PacketPayload::header() const {
		return m_header;
	}

	const std::vector<RawBuffer>& PacketPayload::buffers() const {
		return m_buffers;
	}

	PacketPayload PacketPayload::Merge(const std::shared_ptr<const Packet>& pPacket, const PacketPayload& payload) {
		// pPacket should envelop payload
		PacketPayload mergedPayload(pPacket);
		if (payload.unset())
			return mergedPayload;

		mergedPayload.m_header.Size += payload.m_header.Size;

		// add payload header
		auto pChildPacketHeader = std::make_shared<PacketHeader>(payload.m_header);
		mergedPayload.m_entities.push_back(pChildPacketHeader);
		mergedPayload.m_buffers.push_back({ reinterpret_cast<const uint8_t*>(pChildPacketHeader.get()), sizeof(PacketHeader) });

		// add payload buffers
		mergedPayload.m_entities.insert(mergedPayload.m_entities.end(), payload.m_entities.cbegin(), payload.m_entities.cend());
		mergedPayload.m_buffers.insert(mergedPayload.m_buffers.end(), payload.m_buffers.cbegin(), payload.m_buffers.cend());
		return mergedPayload;
	}
}}
