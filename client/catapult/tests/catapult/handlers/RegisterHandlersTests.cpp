#include "catapult/handlers/RegisterHandlers.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/TestHarness.h"

using namespace catapult::io;

namespace catapult { namespace handlers {

	namespace {
		void AssertHandlerPresent(const ionet::ServerPacketHandlers& handlers, ionet::PacketType packetType) {
			ionet::Packet packet;
			packet.Size = sizeof(ionet::Packet);
			packet.Type = packetType;

			EXPECT_TRUE(handlers.canProcess(packet));
		}
	}

	TEST(RegisterHandlersTests, RegisterAllHandlersRegistersExpectedNumberOfHandlers) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		io::BlockStorageCache storage(std::make_unique<mocks::MemoryBasedStorage>());
		model::TransactionRegistry registry;
		HandlersConfiguration config;

		// Act:
		RegisterAllHandlers(handlers, storage, registry, config);

		// Assert:
		EXPECT_EQ(7u, handlers.size());
		AssertHandlerPresent(handlers, ionet::PacketType::Push_Block);
		AssertHandlerPresent(handlers, ionet::PacketType::Pull_Block);

		AssertHandlerPresent(handlers, ionet::PacketType::Chain_Info);
		AssertHandlerPresent(handlers, ionet::PacketType::Block_Hashes);
		AssertHandlerPresent(handlers, ionet::PacketType::Pull_Blocks);

		AssertHandlerPresent(handlers, ionet::PacketType::Push_Transactions);
		AssertHandlerPresent(handlers, ionet::PacketType::Pull_Transactions);
	}

	TEST(RegisterHandlersTests, RegisterDiagnosticHandlersRegistersExpectedNumberOfHandlers) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		DiagnosticHandlersConfiguration config;

		// Act:
		RegisterDiagnosticHandlers(handlers, config);

		// Assert:
		EXPECT_EQ(1u, handlers.size());
		AssertHandlerPresent(handlers, ionet::PacketType::Diagnostic_Counters);
	}
}}
