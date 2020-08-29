/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "BasicBatchHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region PullHandlerTests

	/// Container of pull handler tests.
	/// \note Tests only test invalid and empty packets. Real response tests are suite-specific.
	template<typename TTraits>
	struct PullHandlerTests : public BasicBatchHandlerTests<TTraits> {
	private:
		using BaseType = BasicBatchHandlerTests<TTraits>;

	public:
		/// Asserts that a valid packet with a payload composed of no elements is accepted.
		static void AssertValidPacketWithEmptyPayloadIsAccepted() {
			BaseType::AssertValidPacketWithElementsIsAccepted(0, TTraits::Data_Header_Size);
		}

		/// Asserts that a valid packet with a payload composed of some elements is accepted.
		static void AssertValidPacketWithPayloadIsAccepted() {
			BaseType::AssertValidPacketWithElementsIsAccepted(3, TTraits::Data_Header_Size);
		}
	};

	// endregion

	// region PullEntitiesHandlerAssertAdapter

	/// Assert for use with DEFINE_PULL_HANDLER_REQUEST_RESPONSE_TESTS for pull handlers implemented with PullEntitiesHandler.
	template<typename TTraits>
	class PullEntitiesHandlerAssertAdapter {
	private:
		using FilterType = typename TTraits::FilterType;

	public:
		static void AssertFunc(uint32_t numRequestHashes, uint32_t numResponseEntities) {
			// Arrange:
			auto packetType = TTraits::Packet_Type;
			auto shortHashesSize = numRequestHashes * SizeOf32<utils::ShortHash>();
			auto pPacket = test::CreateRandomPacket(SizeOf32<FilterType>() + shortHashesSize, packetType);
			ionet::ServerPacketHandlers handlers;
			size_t counter = 0;

			auto extractedRequestData = ExtractRequestParametersFromPacket(*pPacket, numRequestHashes);
			FilterType actualFilterValue;
			utils::ShortHashesSet actualRequestHashes;
			typename TTraits::PullResponseContext responseContext(numResponseEntities);
			TTraits::RegisterHandler(handlers, [&](auto filterValue, const auto& requestHashes) {
				++counter;
				actualFilterValue = filterValue;
				actualRequestHashes = requestHashes;
				return responseContext.response();
			});

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: the requested values were passed to the supplier
			EXPECT_EQ(extractedRequestData.first, actualFilterValue);
			EXPECT_EQ(extractedRequestData.second, actualRequestHashes);

			// - the handler was called and has the correct header
			EXPECT_EQ(1u, counter);
			ASSERT_TRUE(handlerContext.hasResponse());
			auto payload = handlerContext.response();
			test::AssertPacketHeader(payload, sizeof(ionet::PacketHeader) + responseContext.responseSize(), packetType);

			// - let the traits assert the returned payload (may be one or more buffers)
			responseContext.assertPayload(payload);
		}

	private:
		static auto ExtractRequestParametersFromPacket(const ionet::Packet& packet, size_t numRequestHashes) {
			auto filterValue = reinterpret_cast<const FilterType&>(*packet.Data());

			utils::ShortHashesSet extractedShortHashes;
			auto pShortHashData = reinterpret_cast<const utils::ShortHash*>(packet.Data() + sizeof(FilterType));
			for (auto i = 0u; i < numRequestHashes; ++i)
				extractedShortHashes.insert(*pShortHashData++);

			return std::make_pair(filterValue, extractedShortHashes);
		}
	};

	// endregion

#define MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, TEST_NAME) \
	TEST(TEST_CLASS, HANDLER_NAME##_##TEST_NAME) { test::PullHandlerTests<HANDLER_NAME##Traits>::Assert##TEST_NAME(); }

#define DEFINE_PULL_HANDLER_EDGE_CASE_TESTS(TEST_CLASS, HANDLER_NAME) \
	MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, TooSmallPacketIsRejected) \
	MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, PacketWithWrongTypeIsRejected) \
	MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, PacketWithInvalidPayloadIsRejected) \
	\
	MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, ValidPacketWithEmptyPayloadIsAccepted) \
	MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, ValidPacketWithPayloadIsAccepted)

#define MAKE_PULL_HANDLER_REQUEST_RESPONSE_TEST(TEST_CLASS, HANDLER_NAME, ASSERT_FUNC, TEST_NAME, NUM_REQUESTS, NUM_RESPONSES) \
	TEST(TEST_CLASS, HANDLER_NAME##_BehavesCorrectlyWhen##TEST_NAME) { ASSERT_FUNC(NUM_REQUESTS, NUM_RESPONSES); }

#define DEFINE_PULL_HANDLER_REQUEST_RESPONSE_TESTS(TEST_CLASS, HANDLER_NAME, ASSERT_FUNC) \
	MAKE_PULL_HANDLER_REQUEST_RESPONSE_TEST(TEST_CLASS, HANDLER_NAME, ASSERT_FUNC, ZeroResponseEntities, 5, 0) \
	MAKE_PULL_HANDLER_REQUEST_RESPONSE_TEST(TEST_CLASS, HANDLER_NAME, ASSERT_FUNC, SingleResponseEntities, 3, 1) \
	MAKE_PULL_HANDLER_REQUEST_RESPONSE_TEST(TEST_CLASS, HANDLER_NAME, ASSERT_FUNC, MoreRequestHashesThanResponseEntities, 5, 3) \
	MAKE_PULL_HANDLER_REQUEST_RESPONSE_TEST(TEST_CLASS, HANDLER_NAME, ASSERT_FUNC, LessRequestHashesThanResponseEntities, 7, 10)
}}
