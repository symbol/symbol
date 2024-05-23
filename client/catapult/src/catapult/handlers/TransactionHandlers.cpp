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

#include "TransactionHandlers.h"
#include "HandlerUtils.h"

namespace catapult { namespace handlers {

	void RegisterPushTransactionsHandler(
			ionet::ServerPacketHandlers& handlers,
			const model::TransactionRegistry& registry,
			const TransactionRangeHandler& transactionRangeHandler) {
		handlers.registerHandler(
				ionet::PacketType::Push_Transactions,
				CreatePushEntityHandler<model::Transaction>(registry, transactionRangeHandler));
	}

	namespace {
#pragma pack(push, 1)

		struct TransactionsFilter {
			Timestamp Deadline;
			BlockFeeMultiplier FeeMultiplier;
		};

#pragma pack(pop)
	}

	void RegisterPullTransactionsHandler(ionet::ServerPacketHandlers& handlers, const UtRetriever& utRetriever) {
		constexpr auto Packet_Type = ionet::PacketType::Pull_Transactions;
		handlers.registerHandler(
				Packet_Type,
				PullEntitiesHandler<TransactionsFilter>::Create(Packet_Type, [utRetriever](const auto& filter, const auto& shortHashes) {
					return utRetriever(filter.Deadline, filter.FeeMultiplier, shortHashes);
				}));
	}
}}
