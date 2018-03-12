#pragma once
#include "BasicBatchHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// A container of pull handler tests.
	template<typename TTraits>
	struct PullHandlerTests : public BasicBatchHandlerTests<TTraits> {
	private:
		using BaseType = BasicBatchHandlerTests<TTraits>;

	public:
		/// Asserts that a valid packet with an empty payload is accepted.
		static void AssertValidPacketWithEmptyPayloadIsAccepted() {
			BaseType::AssertValidPacketWithElementsIsAccepted(0);
		}

		/// Asserts that a valid packet with a non-empty payload is accepted.
		static void AssertValidPacketWithNonEmptyPayloadIsAccepted() {
			BaseType::AssertValidPacketWithElementsIsAccepted(3);
		}

		/// Asserts that an expected response is set if the request packet is valid.
		static void AssertResponseIsSetIfPacketIsValid() {
			// Assert:
			// - returning 0 or 1 items is handled correctly
			AssertResponseIsSetIfPacketIsValid(5, 0);
			AssertResponseIsSetIfPacketIsValid(3, 1);

			// - returning some items is handled correctly
			AssertResponseIsSetIfPacketIsValid(5, 3);

			// - returning more items than passed in request is handled correctly
			AssertResponseIsSetIfPacketIsValid(7, 10);
		}

	private:
		static void AssertResponseIsSetIfPacketIsValid(uint32_t numRequestEntities, uint32_t numResponseEntities) {
			// Arrange:
			auto pPacket = test::CreateRandomPacket(numRequestEntities * TTraits::Valid_Request_Payload_Size, TTraits::Packet_Type);
			ionet::ServerPacketHandlers handlers;
			size_t counter = 0;

			auto extractedRequestEntities = TTraits::ExtractFromPacket(*pPacket, numRequestEntities);
			typename TTraits::RetrieverParamType actualRequestEntities;
			typename TTraits::ResponseContext responseContext(numResponseEntities);
			TTraits::Register_Handler_Func(handlers, [&](const auto& requestEntities) {
				++counter;
				actualRequestEntities = requestEntities;
				return responseContext.response();
			});

			// Act:
			ionet::ServerPacketHandlerContext context({}, "");
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert: the requested entities were passed to the supplier
			EXPECT_EQ(extractedRequestEntities, actualRequestEntities);

			// - the handler was called and has the correct header
			EXPECT_EQ(1u, counter);
			ASSERT_TRUE(context.hasResponse());
			auto payload = context.response();
			test::AssertPacketHeader(payload, sizeof(ionet::PacketHeader) + responseContext.responseSize(), TTraits::Packet_Type);

			// - let the traits assert the returned payload (may be one or more buffers)
			responseContext.assertPayload(payload);
		}
	};

#define MAKE_PULL_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, TEST_NAME) \
	TEST(TEST_CLASS, HANDLER_NAME##_##TEST_NAME) { test::PullHandlerTests<HANDLER_NAME##Traits>::Assert##TEST_NAME(); }

#define DEFINE_PULL_HANDLER_TESTS(TEST_CLASS, HANDLER_NAME) \
	MAKE_PULL_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, TooSmallPacketIsRejected) \
	MAKE_PULL_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, PacketWithWrongTypeIsRejected) \
	MAKE_PULL_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, PacketWithInvalidPayloadIsRejected) \
	\
	MAKE_PULL_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, ValidPacketWithEmptyPayloadIsAccepted) \
	MAKE_PULL_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, ValidPacketWithNonEmptyPayloadIsAccepted) \
	\
	MAKE_PULL_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, ResponseIsSetIfPacketIsValid)
}}
