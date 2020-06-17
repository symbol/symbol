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

#include "CosignatureHandlers.h"
#include "catapult/handlers/HandlerUtils.h"
#include "catapult/ionet/PacketEntityUtils.h"

namespace catapult { namespace handlers {

	namespace {
		auto CreatePushCosignaturesHandler(const CosignatureRangeHandler& rangeHandler) {
			return [rangeHandler](const ionet::Packet& packet, const auto& context) {
				auto range = ionet::ExtractFixedSizeStructuresFromPacket<model::DetachedCosignature>(packet);
				if (range.empty()) {
					CATAPULT_LOG(warning) << "rejecting empty range: " << packet;
					return;
				}

				CATAPULT_LOG(trace) << "received valid " << packet;
				rangeHandler({ std::move(range), { context.key(), context.host() } });
			};
		}
	}

	void RegisterPushCosignaturesHandler(ionet::ServerPacketHandlers& handlers, const CosignatureRangeHandler& cosignatureRangeHandler) {
		handlers.registerHandler(ionet::PacketType::Push_Detached_Cosignatures, CreatePushCosignaturesHandler(cosignatureRangeHandler));
	}
}}
