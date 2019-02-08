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

#include "TransactionHandlers.h"
#include "HandlerUtils.h"
#include "catapult/ionet/PacketPayloadFactory.h"
#include "catapult/utils/ShortHash.h"
#include "catapult/types.h"

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
		struct PullTransactionsInfo {
		public:
			PullTransactionsInfo() : IsValid(false)
			{}

		public:
			bool IsValid;
			BlockFeeMultiplier MinFeeMultiplier;
			utils::ShortHashesSet ShortHashes;
		};

		auto ProcessPullTransactionsRequest(const ionet::Packet& packet) {
			// packet is guaranteed to have correct type because this function is only called for matching packets by ServerPacketHandlers
			auto dataSize = ionet::CalculatePacketDataSize(packet);
			if (dataSize < sizeof(BlockFeeMultiplier))
				return PullTransactionsInfo();

			// data is prepended with min fee multiplier
			PullTransactionsInfo info;
			info.MinFeeMultiplier = BlockFeeMultiplier(reinterpret_cast<const BlockFeeMultiplier::ValueType&>(*packet.Data()));
			dataSize -= sizeof(BlockFeeMultiplier);

			// followed by short hashes
			const auto* pShortHashDataStart = packet.Data() + sizeof(BlockFeeMultiplier);
			auto numShortHashes = ionet::CountFixedSizeStructures<utils::ShortHash>({ pShortHashDataStart, dataSize });
			if (0 == numShortHashes && 0 != dataSize)
				return PullTransactionsInfo();

			const auto* pShortHash = reinterpret_cast<const utils::ShortHash*>(pShortHashDataStart);
			info.ShortHashes.reserve(numShortHashes);
			for (auto i = 0u; i < numShortHashes; ++i, ++pShortHash)
				info.ShortHashes.insert(*pShortHash);

			info.IsValid = true;
			return info;
		}

		auto CreatePullTransactionsHandler(const UtRetriever& utRetriever) {
			return [utRetriever](const auto& packet, auto& context) {
				auto info = ProcessPullTransactionsRequest(packet);
				if (!info.IsValid)
					return;

				auto transactions = utRetriever(info.MinFeeMultiplier, info.ShortHashes);
				context.response(ionet::PacketPayloadFactory::FromEntities(ionet::PacketType::Pull_Transactions, transactions));
			};
		}
	}

	void RegisterPullTransactionsHandler(ionet::ServerPacketHandlers& handlers, const UtRetriever& utRetriever) {
		handlers.registerHandler(ionet::PacketType::Pull_Transactions, CreatePullTransactionsHandler(utRetriever));
	}
}}
