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

#define MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, TEST_NAME) \
	TEST(TEST_CLASS, HANDLER_NAME##_##TEST_NAME) { test::PullHandlerTests<HANDLER_NAME##Traits>::Assert##TEST_NAME(); }

#define DEFINE_PULL_HANDLER_EDGE_CASE_TESTS(TEST_CLASS, HANDLER_NAME) \
	MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, TooSmallPacketIsRejected) \
	MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, PacketWithWrongTypeIsRejected) \
	MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, PacketWithInvalidPayloadIsRejected) \
	\
	MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, ValidPacketWithEmptyPayloadIsAccepted) \
	MAKE_PULL_HANDLER_EDGE_CASE_TEST(TEST_CLASS, HANDLER_NAME, ValidPacketWithPayloadIsAccepted)

#define MAKE_PULL_HANDLER_REQUEST_RESPONSE_TEST(TEST_CLASS, ASSERT_FUNC, TEST_NAME, NUM_REQUESTS, NUM_RESPONSES) \
	TEST(TEST_CLASS, PullHandlerBehavesCorrectlyWhen##TEST_NAME) { ASSERT_FUNC(NUM_REQUESTS, NUM_RESPONSES); }

#define DEFINE_PULL_HANDLER_REQUEST_RESPONSE_TESTS(TEST_CLASS, ASSERT_FUNC) \
	MAKE_PULL_HANDLER_REQUEST_RESPONSE_TEST(TEST_CLASS, ASSERT_FUNC, ZeroResponseTransactions, 5, 0) \
	MAKE_PULL_HANDLER_REQUEST_RESPONSE_TEST(TEST_CLASS, ASSERT_FUNC, SingleResponseTransactions, 3, 1) \
	MAKE_PULL_HANDLER_REQUEST_RESPONSE_TEST(TEST_CLASS, ASSERT_FUNC, MoreRequestHashesThanResponseTransactions, 5, 3) \
	MAKE_PULL_HANDLER_REQUEST_RESPONSE_TEST(TEST_CLASS, ASSERT_FUNC, LessRequestHashesThanResponseTransactions, 7, 10)
}}
