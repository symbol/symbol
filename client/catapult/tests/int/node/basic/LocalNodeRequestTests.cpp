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

#include "tests/int/node/test/LocalNodeApiTraits.h"
#include "tests/int/node/test/LocalNodeRequestTestUtils.h"
#include "tests/int/node/test/PeerLocalNodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeRequestTests

	namespace {
		using TestContext = test::PeerLocalNodeTestContext;
	}

	// region push

	TEST(TEST_CLASS, CanPushBlockToLocalNode) {
		// Arrange:
		TestContext context;
		test::WaitForBoot(context);

		// Sanity:
		EXPECT_EQ(Height(1), context.height());

		// Act:
		test::ExternalSourceConnection connection;
		auto pIo = test::PushValidBlock(connection);

		// - wait for the chain height to change and for all height readers to disconnect
		context.waitForHeight(Height(2));
		auto chainHeight = context.height();
		WAIT_FOR_ONE_EXPR(context.stats().NumActiveReaders);

		// Assert: the chain height is two
		EXPECT_EQ(Height(2), chainHeight);

		// - a single block element was added
		auto stats = context.stats();
		EXPECT_EQ(1u, stats.NumAddedBlockElements);
		EXPECT_EQ(0u, stats.NumAddedTransactionElements);

		// - the connection is still active
		context.assertSingleReaderConnection();
	}

	TEST(TEST_CLASS, CanPushTransactionToLocalNode) {
		// Arrange:
		TestContext context;
		test::WaitForBoot(context);

		// Act:
		test::ExternalSourceConnection connection;
		auto pIo = test::PushValidTransaction(connection);
		WAIT_FOR_ONE_EXPR(context.stats().NumAddedTransactionElements);

		// Assert: a single transaction element was added
		auto stats = context.stats();
		EXPECT_EQ(0u, stats.NumAddedBlockElements);
		EXPECT_EQ(1u, stats.NumAddedTransactionElements);

		// - the connection is still active
		context.assertSingleReaderConnection();
	}

	// endregion

	// region pull

	CHAIN_API_INT_VALID_TRAITS_BASED_TEST(CanGetResponse) {
		// Assert: the connection is still active
		test::AssertCanGetResponse<TApiTraits>(TestContext(), TestContext::AssertSingleReaderConnection);
	}

	CHAIN_API_INT_INVALID_TRAITS_BASED_TEST(InvalidRequestTriggersException) {
		// Assert: the connection is still active
		test::AssertInvalidRequestTriggersException(
				TestContext(),
				TApiTraits::InitiateInvalidRequest,
				TestContext::AssertSingleReaderConnection);
	}

	// endregion
}}
