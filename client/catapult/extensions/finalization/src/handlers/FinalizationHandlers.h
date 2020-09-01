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
#include "FinalizationHandlerTypes.h"
#include "catapult/ionet/PacketHandlers.h"

namespace catapult { namespace handlers {

	/// Prototype for a function that retrieves messages given a set of short hashes.
	using MessagesRetriever = std::function<std::vector<std::shared_ptr<const model::FinalizationMessage>> (
			FinalizationPoint,
			const utils::ShortHashesSet&)>;

	/// Registers a push messages handler in \a handlers that forwards messages to \a messageRangeHandler.
	void RegisterPushMessagesHandler(ionet::ServerPacketHandlers& handlers, const MessageRangeHandler& messageRangeHandler);

	/// Registers a pull transactions handler in \a handlers that responds with messages
	/// returned by the retriever (\a messagesRetriever).
	void RegisterPullMessagesHandler(ionet::ServerPacketHandlers& handlers, const MessagesRetriever& messagesRetriever);
}}
