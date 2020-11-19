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
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/tree/PatriciaTreeSerializer.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace handlers {

	/// Registers a handler in \a handlers that responds with serialized state path produced by querying \a cache.
	template<typename TPacket, typename TCache>
	void RegisterStatePathHandler(ionet::ServerPacketHandlers& handlers, const TCache& cache) {
		handlers.registerHandler(TPacket::Packet_Type, [&cache](const auto& packet, auto& context) {
			const auto* pRequest = ionet::CoercePacket<TPacket>(&packet);
			if (!pRequest)
				return;

			auto view = cache.createView();
			std::vector<tree::TreeNode> path;
			view->tryLookup(pRequest->Key, path);

			// serialize path even if lookup failed (to provide proof that key does not exist in state)
			std::vector<uint8_t> serializedPath;
			for (const auto& node : path) {
				auto serializedNode = tree::PatriciaTreeSerializer::SerializeValue(node);
				const auto* pData = reinterpret_cast<const uint8_t*>(serializedNode.data());
				serializedPath.insert(serializedPath.end(), pData, pData + serializedNode.size());
			}

			auto payloadSize = utils::checked_cast<size_t, uint32_t>(serializedPath.size());
			auto pResponsePacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
			pResponsePacket->Type = TPacket::Packet_Type;
			utils::memcpy_cond(pResponsePacket->Data(), serializedPath.data(), serializedPath.size());
			context.response(ionet::PacketPayload(pResponsePacket));
		});
	}
}}
