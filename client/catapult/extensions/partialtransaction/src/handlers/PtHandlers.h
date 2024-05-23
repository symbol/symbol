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
#include "catapult/handlers/HandlerTypes.h"
#include "catapult/ionet/PacketHandlers.h"
#include "partialtransaction/src/PtTypes.h"

namespace catapult {
namespace handlers {

	/// Registers a push partial transactions handler in \a handlers that forwards transactions to \a transactionRangeHandler
	/// given a transaction \a registry composed of known transactions.
	void RegisterPushPartialTransactionsHandler(
		ionet::ServerPacketHandlers& handlers,
		const model::TransactionRegistry& registry,
		const TransactionRangeHandler& transactionRangeHandler);

	/// Registers a pull partial transactions handler in \a handlers that responds with partial transactions
	/// returned by the retriever (\a transactionInfosRetriever).
	void RegisterPullPartialTransactionInfosHandler(
		ionet::ServerPacketHandlers& handlers,
		const partialtransaction::CosignedTransactionInfosRetriever& transactionInfosRetriever);
}
}
