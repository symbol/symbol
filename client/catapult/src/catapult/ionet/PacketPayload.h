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

#pragma once
#include "Packet.h"
#include "catapult/types.h"
#include <vector>

namespace catapult { namespace ionet {

	/// Packet payload that can be written.
	class PacketPayload {
	public:
		/// Creates a default (empty) packet payload.
		PacketPayload();

		/// Creates a data-less packet payload with the specified \a type.
		explicit PacketPayload(PacketType type);

		/// Creates a packet payload around a single shared packet (\a pPacket).
		explicit PacketPayload(const std::shared_ptr<const Packet>& pPacket);

	public:
		/// Returns \c true if this packet payload is unset.
		bool unset() const;

		/// Packet header.
		const PacketHeader& header() const;

		/// Packet data.
		const std::vector<RawBuffer>& buffers() const;

	public:
		/// Merges a packet (\a pPacket) and a packet \a payload into a new packet payload.
		static PacketPayload Merge(const std::shared_ptr<const Packet>& pPacket, const PacketPayload& payload);

	private:
		PacketHeader m_header;
		std::vector<RawBuffer> m_buffers;

		// the backing data
		std::vector<std::shared_ptr<const void>> m_entities;

	private:
		friend class PacketPayloadBuilder;
	};
}}
