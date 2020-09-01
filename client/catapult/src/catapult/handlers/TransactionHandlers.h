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
#include "HandlerTypes.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/model/Transaction.h"
#include "catapult/utils/ShortHash.h"
#include <unordered_set>

namespace catapult { namespace handlers {

	/// Transactions returned by the unconfirmed transactions retriever.
	using UnconfirmedTransactions = std::vector<std::shared_ptr<const model::Transaction>>;

	/// Prototype for a function that retrieves unconfirmed transactions given a set of short hashes.
	using UtRetriever = std::function<UnconfirmedTransactions (BlockFeeMultiplier, const utils::ShortHashesSet&)>;

	/// Registers a push transactions handler in \a handlers that forwards transactions to \a transactionRangeHandler
	/// given a transaction \a registry composed of known transactions.
	void RegisterPushTransactionsHandler(
			ionet::ServerPacketHandlers& handlers,
			const model::TransactionRegistry& registry,
			const TransactionRangeHandler& transactionRangeHandler);

	/// Registers a pull transactions handler in \a handlers that responds with unconfirmed transactions
	/// returned by the retriever (\a utRetriever).
	void RegisterPullTransactionsHandler(ionet::ServerPacketHandlers& handlers, const UtRetriever& utRetriever);
}}
