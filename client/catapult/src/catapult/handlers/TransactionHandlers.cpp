#include "TransactionHandlers.h"
#include "HandlerUtils.h"
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
			utils::ShortHashesSet ShortHashes;
			bool IsValid;
		};

		auto ProcessPullTransactionsRequest(const ionet::Packet& packet) {
			if (ionet::PacketType::Pull_Transactions != packet.Type)
				return PullTransactionsInfo();

			auto range = ionet::ExtractFixedSizeStructuresFromPacket<utils::ShortHash>(packet);
			if (range.empty() && sizeof(ionet::Packet) != packet.Size)
				return PullTransactionsInfo();

			PullTransactionsInfo info;
			info.ShortHashes.reserve(range.size());
			for (const auto& shortHash : range)
				info.ShortHashes.insert(shortHash);

			info.IsValid = true;
			return info;
		}

		auto CreatePullTransactionsHandler(const UtRetriever& utRetriever) {
			return [utRetriever](const auto& packet, auto& context) {
				auto info = ProcessPullTransactionsRequest(packet);
				if (!info.IsValid)
					return;

				auto transactions = utRetriever(info.ShortHashes);
				context.response(ionet::PacketPayload::FromEntities(ionet::PacketType::Pull_Transactions, transactions));
			};
		}
	}

	void RegisterPullTransactionsHandler(ionet::ServerPacketHandlers& handlers, const UtRetriever& utRetriever) {
		handlers.registerHandler(ionet::PacketType::Pull_Transactions, CreatePullTransactionsHandler(utRetriever));
	}
}}
