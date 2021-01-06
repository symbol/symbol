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

#include "PtHandlers.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"
#include "catapult/handlers/HandlerUtils.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/PacketPayloadBuilder.h"
#include "catapult/model/RangeTypes.h"

using namespace catapult::partialtransaction;

namespace catapult { namespace handlers {

	namespace {
		auto CreateFilteringHandler(model::EntityType entityType, const TransactionRangeHandler& nextRangeHandler) {
			return [entityType, nextRangeHandler](auto&& annotatedRange) {
				for (const auto& tx : annotatedRange.Range) {
					if (entityType != tx.Type) {
						CATAPULT_LOG(warning) << "unhandled transaction type in range: " << tx.Type;
						return;
					}
				}

				nextRangeHandler(std::move(annotatedRange));
			};
		}

		struct ParsedPullTransactionsRequest {
		public:
			using FilterType = Timestamp;
			using HashType = cache::ShortHashPair;

		public:
			bool IsValid = false;
			FilterType FilterValue;
			cache::ShortHashPairMap ShortHashPairs;

		public:
			static void SetAll(ParsedPullTransactionsRequest& request, const cache::ShortHashPair* pShortHashPair, size_t count) {
				request.ShortHashPairs.reserve(count);
				for (auto i = 0u; i < count; ++i, ++pShortHashPair)
					request.ShortHashPairs.emplace(pShortHashPair->TransactionShortHash, pShortHashPair->CosignaturesShortHash);
			}
		};

		void AppendZeroBytes(ionet::PacketPayloadBuilder& builder, size_t count) {
			builder.appendValues(std::vector<uint8_t>(count, 0));
		}

		void AppendTransactionInfo(ionet::PacketPayloadBuilder& builder, const model::CosignedTransactionInfo& transactionInfo) {
			using CosignatureRange = model::EntityRange<model::Cosignature>;

			auto numCosignatures = static_cast<uint16_t>(transactionInfo.Cosignatures.size());
			if (transactionInfo.pTransaction) {
				builder.appendValue<uint64_t>(0x8000 | numCosignatures); // align transaction start (2 byte tag, 6 byte pad)
				builder.appendEntity(transactionInfo.pTransaction);
				AppendZeroBytes(builder, utils::GetPaddingSize(transactionInfo.pTransaction->Size, 8)); // align transaction end
			} else {
				builder.appendValue<uint64_t>(numCosignatures); // align cosignatures start (2 byte tag, 6 byte pad)
				builder.appendRange(model::HashRange::CopyFixed(transactionInfo.EntityHash.data(), 1));
			}

			const auto* pCosignaturesData = reinterpret_cast<const uint8_t*>(transactionInfo.Cosignatures.data());
			builder.appendRange(CosignatureRange::CopyFixed(pCosignaturesData, transactionInfo.Cosignatures.size()));
		}

		auto BuildPacket(const CosignedTransactionInfos& transactionInfos) {
			ionet::PacketPayloadBuilder builder(ionet::PacketType::Pull_Partial_Transaction_Infos);
			for (const auto& transactionInfo : transactionInfos)
				AppendTransactionInfo(builder, transactionInfo);

			return builder.build();
		}

		auto CreatePullTransactionsHandler(const CosignedTransactionInfosRetriever& transactionInfosRetriever) {
			return [transactionInfosRetriever](const auto& packet, auto& context) {
				auto request = detail::ParsePullRequest<ParsedPullTransactionsRequest>(packet);
				if (!request.IsValid)
					return;

				auto transactionInfos = transactionInfosRetriever(request.FilterValue, request.ShortHashPairs);
				context.response(BuildPacket(transactionInfos));
			};
		}
	}

	void RegisterPushPartialTransactionsHandler(
			ionet::ServerPacketHandlers& handlers,
			const model::TransactionRegistry& registry,
			const TransactionRangeHandler& transactionRangeHandler) {
		handlers.registerHandler(
				ionet::PacketType::Push_Partial_Transactions,
				CreatePushEntityHandler<model::Transaction>(
						registry,
						CreateFilteringHandler(model::Entity_Type_Aggregate_Bonded, transactionRangeHandler)));
	}

	void RegisterPullPartialTransactionInfosHandler(
			ionet::ServerPacketHandlers& handlers,
			const CosignedTransactionInfosRetriever& transactionInfosRetriever) {
		handlers.registerHandler(
				ionet::PacketType::Pull_Partial_Transaction_Infos,
				CreatePullTransactionsHandler(transactionInfosRetriever));
	}
}}
