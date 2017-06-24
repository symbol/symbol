#include "catapult/handlers/DiagnosticHandlers.h"
#include "catapult/model/DiagnosticCounterValue.h"
#include "catapult/utils/DiagnosticCounter.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS DiagnosticHandlersTests

	// region DiagnosticCountersHandler

	TEST(TEST_CLASS, DiagnosticCountersHandler_DoesNotRespondToMalformedRequest) {
		// Arrange:
		ionet::ServerPacketHandlers handlers;
		std::vector<utils::DiagnosticCounter> counters;
		RegisterDiagnosticCountersHandler(handlers, counters);

		// - malform the packet
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = ionet::PacketType::Diagnostic_Counters;
		++pPacket->Size;

		// Act:
		ionet::ServerPacketHandlerContext context;
		EXPECT_TRUE(handlers.process(*pPacket, context));

		// Assert: malformed packet is ignored
		test::AssertNoResponse(context);
	}

	namespace {
		template<typename TAssertHandlerContext>
		void AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(
				const std::vector<utils::DiagnosticCounter>& counters,
				TAssertHandlerContext assertHandlerContext) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			RegisterDiagnosticCountersHandler(handlers, counters);

			// - create a valid request
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
			pPacket->Type = ionet::PacketType::Diagnostic_Counters;

			// Act:
			ionet::ServerPacketHandlerContext context;
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert: header is correct
			auto expectedPacketSize = sizeof(ionet::PacketHeader) + counters.size() * sizeof(model::DiagnosticCounterValue);
			test::AssertPacketHeader(context, expectedPacketSize, ionet::PacketType::Diagnostic_Counters);

			// - counters are written
			assertHandlerContext(context);
		}
	}

	TEST(TEST_CLASS, DiagnosticCountersHandler_WritesCountsInResponseToValidRequest_ZeroCounters) {
		// Assert:
		AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(
				{},
				[](const auto& context) {
					// Assert:
					EXPECT_TRUE(context.response().buffers().empty());
				});
	}

	TEST(TEST_CLASS, DiagnosticCountersHandler_WritesCountsInResponseToValidRequest_SingleCounter) {
		// Assert:
		AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(
				{ utils::DiagnosticCounter(utils::DiagnosticCounterId(123), []() { return 7; }) },
				[](const auto& context) {
					// Assert:
					const auto* pCounterValue = reinterpret_cast<const model::DiagnosticCounterValue*>(test::GetSingleBufferData(context));
					EXPECT_EQ(123u, pCounterValue->Id);
					EXPECT_EQ(7u, pCounterValue->Value);
				});
	}

	TEST(TEST_CLASS, DiagnosticCountersHandler_WritesCountsInResponseToValidRequest_MultipleCounters) {
		// Assert:
		AssertDiagnosticCountersHandlerWritesCountsInResponseToValidRequest(
				{
					utils::DiagnosticCounter(utils::DiagnosticCounterId(123), []() { return 7; }),
					utils::DiagnosticCounter(utils::DiagnosticCounterId(777), []() { return 88; }),
					utils::DiagnosticCounter(utils::DiagnosticCounterId(225), []() { return 222; }),
				},
				[](const auto& context) {
					// Assert:
					const auto* pCounterValue = reinterpret_cast<const model::DiagnosticCounterValue*>(test::GetSingleBufferData(context));
					EXPECT_EQ(123u, pCounterValue->Id);
					EXPECT_EQ(7u, pCounterValue->Value);

					++pCounterValue;
					EXPECT_EQ(777u, pCounterValue->Id);
					EXPECT_EQ(88u, pCounterValue->Value);

					++pCounterValue;
					EXPECT_EQ(225u, pCounterValue->Id);
					EXPECT_EQ(222u, pCounterValue->Value);
				});
	}

	// endregion
}}
