#include "DiagnosticHandlers.h"
#include "catapult/model/DiagnosticCounterValue.h"
#include "catapult/utils/DiagnosticCounter.h"

namespace catapult { namespace handlers {

	// region DiagnosticCountersHandler

	namespace {
		auto CreateDiagnosticCountersHandler(const std::vector<utils::DiagnosticCounter>& counters) {
			return [counters](const auto& packet, auto& context) {
				if (!ionet::IsPacketValid(packet, ionet::PacketType::Diagnostic_Counters))
					return;

				auto payloadSize = utils::checked_cast<size_t, uint32_t>(counters.size() * sizeof(model::DiagnosticCounterValue));
				auto pResponsePacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				pResponsePacket->Type = ionet::PacketType::Diagnostic_Counters;

				auto* pCounterValue = reinterpret_cast<model::DiagnosticCounterValue*>(pResponsePacket->Data());
				for (const auto& counter : counters) {
					pCounterValue->Id = counter.id().value();
					pCounterValue->Value = counter.value();
					++pCounterValue;
				}

				context.response(pResponsePacket);
			};
		}
	}

	void RegisterDiagnosticCountersHandler(
			ionet::ServerPacketHandlers& handlers,
			const std::vector<utils::DiagnosticCounter>& counters) {
		handlers.registerHandler(ionet::PacketType::Diagnostic_Counters, CreateDiagnosticCountersHandler(counters));
	}

	// endregion
}}
