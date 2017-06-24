#include "catapult/ionet/BufferedPacketIo.h"
#include "tests/test/net/SocketTestUtils.h"

namespace catapult { namespace ionet {

	namespace {
		std::shared_ptr<PacketIo> Transform(
				boost::asio::io_service& service,
				const std::shared_ptr<PacketIo>& pIo) {
			return CreateBufferedPacketIo(pIo, boost::asio::strand(service));
		}
	}

	TEST(BufferedPacketIoTests, WriteCanWriteMultipleConsecutivePayloads) {
		// Assert:
		test::AssertWriteCanWriteMultipleConsecutivePayloads(Transform);
	}

	TEST(BufferedPacketIoTests, WriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving) {
		// Assert:
		test::AssertWriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving(Transform);
	}

	TEST(BufferedPacketIoTests, ReadCanReadMultipleConsecutivePayloads) {
		// Assert:
		test::AssertReadCanReadMultipleConsecutivePayloads(Transform);
	}

	TEST(BufferedPacketIoTests, ReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving) {
		// Assert:
		test::AssertReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving(Transform);
	}
}}
