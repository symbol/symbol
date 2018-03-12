#include "partialtransaction/src/handlers/CosignatureHandler.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/core/PushHandlerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS CosignatureHandlersTests

	namespace {
		struct PushCosignaturesTraits {
			static constexpr auto Packet_Type = ionet::PacketType::Push_Detached_Cosignatures;
			static constexpr auto Data_Size = sizeof(model::DetachedCosignature);

			static constexpr size_t AdditionalPacketSize(size_t) {
				return 0u;
			}

			static void PreparePacket(ionet::ByteBuffer&, size_t) {
			}

			static auto CreateRegistry() {
				// note that int is used as a placeholder transaction registy
				// because a real one is not needed by RegisterPushCosignaturesHandler
				return 7;
			}

			static auto RegisterHandler(ionet::ServerPacketHandlers& handlers, int, const CosignatureRangeHandler& rangeHandler) {
				return RegisterPushCosignaturesHandler(handlers, rangeHandler);
			}
		};
	}

	DEFINE_PUSH_HANDLER_TESTS(TEST_CLASS, PushCosignatures)
}}
