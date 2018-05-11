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

#include "tests/test/int/LocalNodeApiTraits.h"
#include "tests/test/int/LocalNodeIntegrityTestUtils.h"
#include "tests/test/int/LocalNodeTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeIntegrityTests

	namespace {
		class TestContext {
		public:
			TestContext() : m_context(test::NodeFlag::With_Partner, { test::CreateLocalPartnerNode() })
			{}

			auto stats() const {
				return m_context.stats();
			}

		private:
			test::LocalNodeTestContext<test::LocalNodePeerTraits> m_context;
		};

		Height RetrieveHeight() {
			test::ExternalSourceConnection connection;

			std::atomic<Height::ValueType> height(0);
			std::atomic_bool gotHeight(false);
			connection.apiCall([&](const auto& pRemoteChainApi) {
				pRemoteChainApi->chainInfo().then([&gotHeight, &height](auto&& infoFuture) {
					height = infoFuture.get().Height.unwrap();
					gotHeight = true;
				});
			});
			WAIT_FOR(gotHeight);
			return Height(height);
		}

		void AssertReaderConnection(const test::PeerLocalNodeStats& stats) {
			// Assert: the external reader connection is still active
			EXPECT_EQ(1u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
			EXPECT_EQ(0u, stats.NumActiveBroadcastWriters);
		}
	}

	// region push

	TEST(TEST_CLASS, CanPushBlockToLocalNode) {
		// Arrange:
		TestContext context;
		test::WaitForBoot(context);

		// Sanity:
		EXPECT_EQ(Height(1), RetrieveHeight());

		// Act:
		test::ExternalSourceConnection connection;
		auto pIo = test::PushValidBlock(connection);

		// - wait for the chain height to change and for all height readers to disconnect
		WAIT_FOR_VALUE_EXPR(Height(2), RetrieveHeight());
		auto chainHeight = RetrieveHeight();
		WAIT_FOR_ONE_EXPR(context.stats().NumActiveReaders);

		// Assert: the chain height is two
		EXPECT_EQ(Height(2), chainHeight);

		// - a single block element was added
		auto stats = context.stats();
		EXPECT_EQ(1u, stats.NumAddedBlockElements);
		EXPECT_EQ(0u, stats.NumAddedTransactionElements);

		// - the connection is still active
		AssertReaderConnection(stats);
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
		AssertReaderConnection(stats);
	}

	// endregion

	// region pull

	CHAIN_API_INT_VALID_TRAITS_BASED_TEST(CanGetResponse) {
		// Assert: the connection is still active
		test::AssertCanGetResponse<TApiTraits>(TestContext(), AssertReaderConnection);
	}

	CHAIN_API_INT_INVALID_TRAITS_BASED_TEST(InvalidRequestTriggersException) {
		// Assert: the connection is still active
		test::AssertInvalidRequestTriggersException(TestContext(), TApiTraits::InitiateInvalidRequest, AssertReaderConnection);
	}

	// endregion
}}
