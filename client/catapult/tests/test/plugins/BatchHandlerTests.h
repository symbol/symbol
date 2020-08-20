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
#include "catapult/handlers/BasicProducer.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Container of batch handler tests.
	template<typename TTraits>
	struct BatchHandlerTests : public BasicBatchHandlerTests<TTraits> {
	private:
		using BaseType = BasicBatchHandlerTests<TTraits>;
		using RequestRange = model::EntityRange<typename TTraits::RequestStructureType>;
		using RequestStructureVector = std::vector<typename TTraits::RequestStructureType>;

	public:
		/// Asserts that a valid packet with a payload composed of some elements is accepted.
		static void AssertValidPacketWithPayloadIsAccepted() {
			// Assert:
			BaseType::AssertValidPacketWithElementsIsAccepted(3);
		}

		/// Asserts that an expected response is set if the request packet is valid.
		static void AssertResponseIsSetWhenPacketIsValid() {
			// Assert:
			// - returning 1 item is handled correctly
			AssertResponseIsSetWhenPacketIsValid(3, 1);

			// - returning some items is handled correctly
			AssertResponseIsSetWhenPacketIsValid(5, 3);

			// - returning more items than passed in request is handled correctly
			AssertResponseIsSetWhenPacketIsValid(7, 10);
		}

	private:
		static void AssertExpectedRequest(const RequestStructureVector& expectedStructures, const RequestRange& actualRange) {
			ASSERT_EQ(expectedStructures.size(), actualRange.size());

			auto i = 0u;
			for (const auto& structure : actualRange) {
				EXPECT_EQ(expectedStructures[i], structure) << TTraits::Message << i;
				++i;
			}
		}

		static void AssertResponseIsSetWhenPacketIsValid(uint32_t numRequestStructures, uint32_t numResponseEntities) {
			// Arrange:
			using RequestStructureType = typename TTraits::RequestStructureType;

			auto pPacket = test::CreateRandomPacket(numRequestStructures * TTraits::Valid_Request_Payload_Size, TTraits::Packet_Type);
			ionet::ServerPacketHandlers handlers;
			size_t counter = 0;

			std::vector<RequestStructureType> extractedRequestStructures;
			const auto* pData = reinterpret_cast<const RequestStructureType*>(pPacket->Data());
			for (auto i = 0u; i < numRequestStructures; ++i)
				extractedRequestStructures.push_back(*pData++);

			typename TTraits::ResponseState responseState;
			auto expectedResponse = TTraits::CreateResponse(numResponseEntities, responseState);
			RequestRange requestStructureRange;
			TTraits::RegisterHandler(handlers, [&](const auto& requestStructures) {
				++counter;
				requestStructureRange = RequestRange::CopyRange(requestStructures);
				return expectedResponse;
			});

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert: the requested structures were passed to the supplier
			AssertExpectedRequest(extractedRequestStructures, requestStructureRange);

			// - the handler was called and has the correct header
			EXPECT_EQ(1u, counter);
			ASSERT_TRUE(handlerContext.hasResponse());
			test::AssertPacketHeader(
					handlerContext,
					sizeof(ionet::PacketHeader) + TTraits::TotalSize(expectedResponse),
					TTraits::Packet_Type);

			// - the entities returned by the results supplier were copied into the response packet as a single buffer
			TTraits::AssertExpectedResponse(handlerContext.response(), expectedResponse);
		}
	};

	/// Adapts a batch handler supplier \a action to a producer.
	template<typename TResponse, typename TAction>
	static auto BatchHandlerSupplierActionToProducer(TAction action) {
		class Producer : public handlers::BasicProducer<TResponse> {
		public:
			using handlers::BasicProducer<TResponse>::BasicProducer;

		public:
			auto operator()() {
				return this->next([](const typename TResponse::value_type& pValue) {
					return pValue;
				});
			}
		};

		return [action](const auto& inputs) {
			auto pResponseValues = std::make_shared<TResponse>(action(inputs)); // used by producer by reference
			auto producer = Producer(*pResponseValues);
			return [pResponseValues, producer]() mutable {
				return producer();
			};
		};
	}

#define MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, TEST_NAME) \
	TEST(TEST_CLASS, HANDLER_NAME##_##TEST_NAME) { test::BatchHandlerTests<HANDLER_NAME##Traits>::Assert##TEST_NAME(); }

#define DEFINE_BATCH_HANDLER_TESTS(TEST_CLASS, HANDLER_NAME) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, TooSmallPacketIsRejected) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, PacketWithWrongTypeIsRejected) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, PacketWithInvalidPayloadIsRejected) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, PacketWithTooSmallPayloadIsRejected) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, PacketWithNoPayloadIsRejected) \
	\
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, ValidPacketWithPayloadIsAccepted) \
	MAKE_BATCH_HANDLER_TEST(TEST_CLASS, HANDLER_NAME, ResponseIsSetWhenPacketIsValid)
}}
