#pragma once
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// A container of batch handler tests.
	template<typename TTraits>
	struct BatchHandlerTests {
	private:
		using RequestEntityRange = model::EntityRange<typename TTraits::RequestEntityType>;

	public:
		/// Asserts that a packet with a reported size less than a packet header is rejected.
		static void AssertTooSmallPacketIsRejected() {
			// Arrange:
			ionet::Packet packet;
			packet.Size = sizeof(ionet::PacketHeader) - 1;
			packet.Type = TTraits::Packet_Type;

			// Assert:
			AssertPacketIsRejected(packet);
		}

		/// Asserts that a packet with the wrong type is rejected.
		static void AssertPacketWithWrongTypeIsRejected() {
			// Assert: wrong packet type
			AssertPacketIsRejected(TTraits::Valid_Payload_Size, ionet::PacketType::Pull_Transactions, false);
		}

		/// Asserts that a packet with an invalid payload size is rejected.
		static void AssertPacketWithInvalidPayloadIsRejected() {
			// Assert: payload size is not divisible by TTraits::ValidPayloadSize
			AssertPacketIsRejected(TTraits::Valid_Payload_Size + TTraits::Valid_Payload_Size / 2, TTraits::Packet_Type);
		}

		/// Asserts that a packet without a payload is rejected.
		static void AssertPacketWithNoPayloadIsRejected() {
			// Assert: no payload
			AssertPacketIsRejected(0, TTraits::Packet_Type);
		}

	public:
		/// Asserts that a packet with a non-empty payload is accepted.
		static void AssertValidPacketWithNonEmptyPayloadIsAccepted() {
			// Arrange:
			size_t counter = 0;
			ionet::ServerPacketHandlers handlers;
			RegisterHandler(handlers, counter);

			auto pPacket = test::CreateRandomPacket(3 * TTraits::Valid_Payload_Size, TTraits::Packet_Type);

			// Act:
			ionet::ServerPacketHandlerContext context;
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert:
			EXPECT_EQ(1u, counter);
			ASSERT_TRUE(context.hasResponse());
			test::AssertPacketHeader(context, sizeof(ionet::PacketHeader), TTraits::Packet_Type);
			EXPECT_TRUE(context.response().buffers().empty());
		}

		/// Asserts that an expected response is set if the request packet is valid.
		static void AssertResponseIsSetIfPacketIsValid() {
			// Assert:
			AssertResponseIsSetIfPacketIsValid(3, 1);
			AssertResponseIsSetIfPacketIsValid(5, 3);
			AssertResponseIsSetIfPacketIsValid(7, 10);
		}

	private:
		static void RegisterHandler(ionet::ServerPacketHandlers& handlers, size_t& counter) {
			TTraits::Register_Handler_Func(handlers, [&](const auto&) {
				++counter;
				return typename TTraits::ResponseType();
			});
		}

		static void RegisterHandler(
				ionet::ServerPacketHandlers& handlers,
				RequestEntityRange& requestEntityRange,
				const typename TTraits::ResponseType& response,
				size_t& counter) {
			TTraits::Register_Handler_Func(handlers, [&](const auto& requestEntities) {
				++counter;
				requestEntityRange = RequestEntityRange::CopyRange(requestEntities);
				return response;
			});
		}

		static void AssertExpectedRequest(
				const RequestEntityRange& actualRange,
				const std::vector<typename TTraits::RequestEntityType>& expectedEntities) {
			ASSERT_EQ(expectedEntities.size(), actualRange.size());

			auto i = 0u;
			for (const auto& entity : actualRange) {
				EXPECT_EQ(expectedEntities[i], entity) << TTraits::Message() << i;
				++i;
			}
		}

		static void AssertPacketIsRejected(const ionet::Packet& packet, bool expectedCanProcessPacketType = true) {
			// Arrange:
			size_t counter = 0;
			ionet::ServerPacketHandlers handlers;
			RegisterHandler(handlers, counter);

			// Act:
			ionet::ServerPacketHandlerContext context;
			EXPECT_EQ(expectedCanProcessPacketType, handlers.process(packet, context));

			// Assert:
			EXPECT_EQ(0u, counter);
			test::AssertNoResponse(context);
		}

		static void AssertPacketIsRejected(uint32_t payloadSize, ionet::PacketType type, bool expectedCanProcessPacketType = true) {
			// Arrange:
			auto pPacket = test::CreateRandomPacket(payloadSize, type);

			// Assert:
			AssertPacketIsRejected(*pPacket, expectedCanProcessPacketType);
		}

		static void AssertResponseIsSetIfPacketIsValid(uint32_t numRequestEntities, uint32_t numResponseEntities) {
			// Arrange:
			using RequestEntityType = typename TTraits::RequestEntityType;

			auto pPacket = test::CreateRandomPacket(numRequestEntities * TTraits::Valid_Payload_Size, TTraits::Packet_Type);
			ionet::ServerPacketHandlers handlers;
			size_t counter = 0;

			std::vector<RequestEntityType> extractedRequestEntities;
			auto pData = reinterpret_cast<const RequestEntityType*>(pPacket->Data());
			for (auto i = 0u; i < numRequestEntities; ++i)
				extractedRequestEntities.push_back(*pData++);

			typename TTraits::ResponseState responseState;
			auto expectedResponse = TTraits::CreateResponse(numResponseEntities, responseState);
			RequestEntityRange requestEntityRange;
			RegisterHandler(handlers, requestEntityRange, expectedResponse, counter);

			// Act:
			ionet::ServerPacketHandlerContext context;
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert: the requested entities were passed to the supplier
			AssertExpectedRequest(requestEntityRange, extractedRequestEntities);

			// - the handler was called and has the correct header
			EXPECT_EQ(1u, counter);
			ASSERT_TRUE(context.hasResponse());
			test::AssertPacketHeader(
					context,
					sizeof(ionet::PacketHeader) + TTraits::TotalSize(expectedResponse),
					TTraits::Packet_Type);

			// - the entities returned by the results supplier were copied into the response packet as a single buffer
			TTraits::AssertExpectedResponse(context.response(), expectedResponse);
		}
	};

#define MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, TEST_NAME) \
	TEST(TEST_CLASS, HANDLER_NAME##_##TEST_NAME) { test::BatchHandlerTests<HANDLER_NAME##Traits>::Assert##TEST_NAME(); }

#define DEFINE_BATCH_HANDLER_TESTS(TEST_CLASS, HANDLER_NAME) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, TooSmallPacketIsRejected) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, PacketWithWrongTypeIsRejected) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, PacketWithInvalidPayloadIsRejected) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, PacketWithNoPayloadIsRejected) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, ValidPacketWithNonEmptyPayloadIsAccepted) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, ResponseIsSetIfPacketIsValid)
}}
