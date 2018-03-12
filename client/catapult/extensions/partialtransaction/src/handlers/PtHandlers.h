#pragma once
#include "partialtransaction/src/PtTypes.h"
#include "catapult/handlers/HandlerTypes.h"
#include "catapult/ionet/PacketHandlers.h"

namespace catapult { namespace handlers {

	/// Registers a push partial transactions handler in \a handlers that forwards transactions to \a transactionRangeHandler
	/// given a \a registry composed of known transactions.
	void RegisterPushPartialTransactionsHandler(
			ionet::ServerPacketHandlers& handlers,
			const model::TransactionRegistry& registry,
			const TransactionRangeHandler& transactionRangeHandler);

	/// Registers a pull partial transactions handler in \a handlers that responds with partial transactions
	/// returned by the retriever (\a transactionInfosRetriever).
	void RegisterPullPartialTransactionInfosHandler(
			ionet::ServerPacketHandlers& handlers,
			const partialtransaction::CosignedTransactionInfosRetriever& transactionInfosRetriever);
}}
