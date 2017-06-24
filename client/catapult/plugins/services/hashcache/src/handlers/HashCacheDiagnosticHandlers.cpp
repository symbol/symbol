#include "HashCacheDiagnosticHandlers.h"
#include "catapult/handlers/HandlerFactory.h"

namespace catapult { namespace handlers {

	namespace {
		struct ConfirmTimestampedHashesTraits {
			using EntityType = state::TimestampedHash;
			using SupplierResultsType = std::vector<const EntityType*>;

			static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Confirm_Timestamped_Hashes;

			static auto ToPayload(const SupplierResultsType& results) {
				auto payloadSize = utils::checked_cast<size_t, uint32_t>(results.size() * sizeof(state::TimestampedHash));
				auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				pPacket->Type = Packet_Type;
				state::TimestampedHash* pData = reinterpret_cast<state::TimestampedHash*>(pPacket->Data());
				for (const auto& pTimestampedHash : results)
					std::memcpy(pData++, pTimestampedHash, sizeof(state::TimestampedHash));

				return pPacket;
			}
		};
	}

	void RegisterConfirmTimestampedHashesHandler(
			ionet::ServerPacketHandlers& handlers,
			const ConfirmedTimestampedHashesFilter& confirmedTimestampedHashesFilter) {
		using HandlerFactory = BatchHandlerFactory<ConfirmTimestampedHashesTraits>;
		handlers.registerHandler(HandlerFactory::Packet_Type, HandlerFactory::Create(confirmedTimestampedHashesFilter));
	}
}}
