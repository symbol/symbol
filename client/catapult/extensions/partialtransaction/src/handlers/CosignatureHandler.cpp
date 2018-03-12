#include "CosignatureHandler.h"
#include "catapult/handlers/HandlerUtils.h"
#include "catapult/ionet/PacketEntityUtils.h"

namespace catapult { namespace handlers {

	namespace {
		auto CreatePushCosignaturesHandler(const CosignatureRangeHandler& rangeHandler) {
			return [rangeHandler](const ionet::Packet& packet, const auto& context) {
				auto range = ionet::ExtractFixedSizeStructuresFromPacket<model::DetachedCosignature>(packet);
				if (range.empty()) {
					CATAPULT_LOG(warning) << "rejecting empty range (packet type = " << packet.Type << ", size " << packet.Size << ")";
					return;
				}

				CATAPULT_LOG(trace) << "received valid packet (type = " << packet.Type << ") with size " << packet.Size;
				rangeHandler({ std::move(range), context.key() });
			};
		}
	}

	void RegisterPushCosignaturesHandler(ionet::ServerPacketHandlers& handlers, const CosignatureRangeHandler& cosignatureRangeHandler) {
		handlers.registerHandler(ionet::PacketType::Push_Detached_Cosignatures, CreatePushCosignaturesHandler(cosignatureRangeHandler));
	}
}}
