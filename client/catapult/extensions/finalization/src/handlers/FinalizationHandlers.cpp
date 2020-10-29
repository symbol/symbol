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

#include "FinalizationHandlers.h"
#include "catapult/handlers/HandlerUtils.h"

namespace catapult { namespace handlers {

	namespace {
		auto CreatePushMessagesHandler(const MessageRangeHandler& rangeHandler) {
			return [rangeHandler](const ionet::Packet& packet, const auto& context) {
				auto range = ionet::ExtractEntitiesFromPacket<model::FinalizationMessage>(
						packet,
						model::IsSizeValidT<model::FinalizationMessage>);
				if (range.empty()) {
					CATAPULT_LOG(warning) << "rejecting empty range: " << packet;
					return;
				}

				CATAPULT_LOG(trace) << "received valid " << packet;
				rangeHandler({ std::move(range), { context.key(), context.host() } });
			};
		}
	}

	void RegisterPushMessagesHandler(ionet::ServerPacketHandlers& handlers, const MessageRangeHandler& messageRangeHandler) {
		handlers.registerHandler(ionet::PacketType::Push_Finalization_Messages, CreatePushMessagesHandler(messageRangeHandler));
	}

	void RegisterPullMessagesHandler(ionet::ServerPacketHandlers& handlers, const MessagesRetriever& messagesRetriever) {
		constexpr auto Packet_Type = ionet::PacketType::Pull_Finalization_Messages;
		handlers.registerHandler(Packet_Type, PullEntitiesHandler<model::FinalizationRoundRange>::Create(Packet_Type, messagesRetriever));
	}
}}
