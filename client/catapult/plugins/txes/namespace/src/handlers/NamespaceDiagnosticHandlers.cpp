#include "NamespaceDiagnosticHandlers.h"
#include "src/model/MosaicInfo.h"
#include "src/model/NamespaceInfo.h"
#include "catapult/handlers/HandlerFactory.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace handlers {

	// region MosaicInfos

	namespace {
		struct MosaicInfosTraits {
			using RequestStructureType = MosaicId;
			using SupplierResultsType = std::vector<std::shared_ptr<const model::MosaicInfo>>;

			static constexpr auto Packet_Type = ionet::PacketType::Mosaic_Infos;

			static auto ToPayload(const SupplierResultsType& results) {
				auto payloadSize = utils::checked_cast<size_t, uint32_t>(results.size() * sizeof(model::MosaicInfo));
				auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				pPacket->Type = Packet_Type;
				model::MosaicInfo* pData = reinterpret_cast<model::MosaicInfo*>(pPacket->Data());
				for (const auto& pMosaicInfo : results)
					std::memcpy(pData++, pMosaicInfo.get(), sizeof(model::MosaicInfo));

				return pPacket;
			}
		};
	}

	void RegisterMosaicInfosHandler(ionet::ServerPacketHandlers& handlers, const MosaicInfosSupplier& mosaicInfosSupplier) {
		using HandlerFactory = BatchHandlerFactory<MosaicInfosTraits>;
		handlers.registerHandler(HandlerFactory::Packet_Type, HandlerFactory::Create(mosaicInfosSupplier));
	}

	// endregion

	// region NamespaceInfos

	namespace {
		struct NamespaceInfosTraits {
			using RequestStructureType = NamespaceId;
			using SupplierResultsType = std::vector<std::shared_ptr<const model::NamespaceInfo>>;

			static constexpr auto Packet_Type = ionet::PacketType::Namespace_Infos;

			static auto ToPayload(const SupplierResultsType& results) {
				return ionet::PacketPayload::FromEntities(Packet_Type, results);
			}
		};
	}

	void RegisterNamespaceInfosHandler(ionet::ServerPacketHandlers& handlers, const NamespaceInfosSupplier& namespaceInfosSupplier) {
		using HandlerFactory = BatchHandlerFactory<NamespaceInfosTraits>;
		handlers.registerHandler(HandlerFactory::Packet_Type, HandlerFactory::Create(namespaceInfosSupplier));
	}

	// endregion
}}
