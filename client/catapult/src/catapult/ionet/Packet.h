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

#pragma once
#include "PacketHeader.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/types.h"
#include <memory>

namespace catapult { namespace ionet {

#pragma pack(push, 1)

	/// Packet header with a data payload.
	struct Packet : public PacketHeader, public utils::NonCopyable {
	public:
		/// Gets a non-const pointer to data contained in this packet.
		uint8_t* Data() {
			return Size <= sizeof(Packet) ? nullptr : reinterpret_cast<uint8_t*>(this) + sizeof(Packet);
		}

		/// Gets a const pointer to data contained in this packet.
		constexpr const uint8_t* Data() const {
			return Size <= sizeof(Packet) ? nullptr : reinterpret_cast<const uint8_t*>(this) + sizeof(Packet);
		}
	};

#pragma pack(pop)

	/// Creates a packet of the specified type (\a TPacket) with the specified payload size.
	template<typename TPacket>
	std::shared_ptr<TPacket> CreateSharedPacket(uint32_t payloadSize = 0) {
		uint32_t packetSize = SizeOf32<TPacket>() + payloadSize;
		auto pPacket = utils::MakeSharedWithSize<TPacket>(packetSize);
		pPacket->Size = packetSize;
		pPacket->Type = TPacket::Packet_Type;
		return pPacket;
	}

	template<>
	inline std::shared_ptr<Packet> CreateSharedPacket(uint32_t payloadSize) {
		uint32_t packetSize = SizeOf32<Packet>() + payloadSize;
		auto pPacket = utils::MakeSharedWithSize<Packet>(packetSize);
		pPacket->Size = packetSize;
		pPacket->Type = PacketType::Undefined;
		return pPacket;
	}

	/// Coerces \a pPacket to the desired packet type or \c nullptr if it is incompatible.
	template<typename TPacket>
	const TPacket* CoercePacket(const Packet* pPacket) {
		return TPacket::Packet_Type != pPacket->Type || sizeof(TPacket) != pPacket->Size
			? nullptr
			: static_cast<const TPacket*>(pPacket);
	}

	/// Checks if \a packet is valid with \a type.
	constexpr bool IsPacketValid(const Packet& packet, PacketType type) {
		return sizeof(Packet) == packet.Size && type == packet.Type;
	}
}}
