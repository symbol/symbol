#include "PtHandlers.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"
#include "catapult/handlers/HandlerUtils.h"
#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/PacketPayload.h"
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

		struct PullTransactionsInfo {
		public:
			PullTransactionsInfo() : IsValid(false)
			{}

		public:
			cache::ShortHashPairMap ShortHashPairs;
			bool IsValid;
		};

		auto ProcessPullTransactionsRequest(const ionet::Packet& packet) {
			if (ionet::PacketType::Pull_Partial_Transaction_Infos != packet.Type)
				return PullTransactionsInfo();

			auto range = ionet::ExtractFixedSizeStructuresFromPacket<cache::ShortHashPair>(packet);
			if (range.empty() && sizeof(ionet::Packet) != packet.Size)
				return PullTransactionsInfo();

			PullTransactionsInfo info;
			info.ShortHashPairs.reserve(range.size());
			for (const auto& hashPair : range)
				info.ShortHashPairs.emplace(hashPair.TransactionShortHash, hashPair.CosignaturesShortHash);

			info.IsValid = true;
			return info;
		}

		void AppendTransactionInfo(ionet::PacketPayloadBuilder& builder, const model::CosignedTransactionInfo& transactionInfo) {
			using CosignatureRange = model::EntityRange<model::Cosignature>;

			auto numCosignatures = static_cast<uint16_t>(transactionInfo.Cosignatures.size());
			if (transactionInfo.pTransaction) {
				builder.appendValue<uint16_t>(0x8000 | numCosignatures);
				builder.appendEntity(transactionInfo.pTransaction);
			} else {
				builder.appendValue<uint16_t>(numCosignatures);
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
				auto info = ProcessPullTransactionsRequest(packet);
				if (!info.IsValid)
					return;

				auto transactionInfos = transactionInfosRetriever(info.ShortHashPairs);
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
