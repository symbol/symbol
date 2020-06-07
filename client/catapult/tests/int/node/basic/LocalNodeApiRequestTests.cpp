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
#include "tests/int/node/test/LocalNodeTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeApiRequestTests

	namespace {
		class TestContext {
		public:
			TestContext() : m_context(test::NodeFlag::With_Partner | test::NodeFlag::Simulated_Api, {})
			{}

		public:
			auto publicKey() const {
				return m_context.publicKey();
			}

			auto stats() const {
				return m_context.stats();
			}

		private:
			test::LocalNodeTestContext<test::LocalNodeApiTraits> m_context;
		};

		void AssertNoReaderConnection(const test::BasicLocalNodeStats& stats) {
			// Assert: the external reader connection is not active
			EXPECT_EQ(0u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
		}
	}

	// region push

	TEST(TEST_CLASS, CannotPushBlockToLocalNode) {
		// Arrange:
		TestContext context;
		test::WaitForBoot(context);

		// Act:
		// - note that push valid block will create a new reader connection, increasing the number of readers from 1 (self) to 2
		test::ExternalSourceConnection connection(context.publicKey());
		auto pIo = test::PushValidBlock(connection);

		// - wait for the external reader to be closed by the server
		WAIT_FOR_ZERO_EXPR(context.stats().NumActiveReaders);

		// Assert: no block element was added
		auto stats = context.stats();
		EXPECT_EQ(0u, stats.NumAddedBlockElements);
		EXPECT_EQ(0u, stats.NumAddedTransactionElements);

		// - the connection is no longer active
		AssertNoReaderConnection(stats);
	}

	TEST(TEST_CLASS, CanPushTransactionToLocalNode) {
		// Arrange:
		TestContext context;
		test::WaitForBoot(context);

		// Act:
		test::ExternalSourceConnection connection(context.publicKey());
		auto pIo = test::PushValidTransaction(connection);
		WAIT_FOR_ONE_EXPR(context.stats().NumAddedTransactionElements);

		// Assert: a single transaction element was added
		auto stats = context.stats();
		EXPECT_EQ(0u, stats.NumAddedBlockElements);
		EXPECT_EQ(1u, stats.NumAddedTransactionElements);

		// - the connection is no longer active because after sending the transaction, PushPayload (called by PushValidTransaction)
		//   initiates a Chain_Info request, which is not supported and causes the connection to be closed
		AssertNoReaderConnection(stats);
	}

	// endregion

	// region pull (unsupported)

	CHAIN_API_INT_VALID_TRAITS_BASED_TEST(CannotGetResponse) {
		// Assert: the connection is no longer active
		test::AssertInvalidRequestTriggersException(TestContext(), TApiTraits::InitiateValidRequest, AssertNoReaderConnection);
	}

	// endregion
}}
