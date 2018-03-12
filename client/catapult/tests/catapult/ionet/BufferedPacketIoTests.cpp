#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/PacketSocket.h"
#include "tests/test/net/SocketTestUtils.h"

namespace catapult { namespace ionet {

#define TEST_CLASS BufferedPacketIoTests

	namespace {
		std::shared_ptr<PacketIo> Transform(const std::shared_ptr<PacketSocket>& pSocket) {
			return pSocket->buffered();
		}
	}

	TEST(TEST_CLASS, WriteCanWriteMultipleConsecutivePayloads) {
		// Assert:
		test::AssertWriteCanWriteMultipleConsecutivePayloads(Transform);
	}

	TEST(TEST_CLASS, WriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving) {
		// Assert:
		test::AssertWriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving(Transform);
	}

	TEST(TEST_CLASS, ReadCanReadMultipleConsecutivePayloads) {
		// Assert:
		test::AssertReadCanReadMultipleConsecutivePayloads(Transform);
	}

	TEST(TEST_CLASS, ReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving) {
		// Assert:
		test::AssertReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving(Transform);
	}
}}
