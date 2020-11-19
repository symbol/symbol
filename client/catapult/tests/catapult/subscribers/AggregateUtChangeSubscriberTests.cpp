/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/subscribers/AggregateUtChangeSubscriber.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/catapult/subscribers/test/UnsupportedSubscribers.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS AggregateUtChangeSubscriberTests

	namespace {
		using UnsupportedUtChangeSubscriber = test::UnsupportedUtChangeSubscriber<test::UnsupportedFlushBehavior::Throw>;

		template<typename TUtChangeSubscriber>
		using TestContext = test::AggregateSubscriberTestContext<TUtChangeSubscriber, AggregateUtChangeSubscriber<TUtChangeSubscriber>>;
	}

	TEST(TEST_CLASS, NotifyAddsForwardsToAllSubscribers) {
		// Arrange:
		DEFINE_MOCK_INFOS_CAPTURE(UtChangeSubscriber, notifyAdds);

		TestContext<MockUtChangeSubscriber> context;
		auto transactionInfos = test::CopyTransactionInfosToSet(test::CreateTransactionInfos(3));

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyAdds(transactionInfos);

		// Assert:
		test::AssertInfosDelegation(context, transactionInfos);
	}

	TEST(TEST_CLASS, NotifyRemovesForwardsToAllSubscribers) {
		// Arrange:
		DEFINE_MOCK_INFOS_CAPTURE(UtChangeSubscriber, notifyRemoves);

		TestContext<MockUtChangeSubscriber> context;
		auto transactionInfos = test::CopyTransactionInfosToSet(test::CreateTransactionInfos(3));

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyRemoves(transactionInfos);

		// Assert:
		test::AssertInfosDelegation(context, transactionInfos);
	}

	TEST(TEST_CLASS, FlushForwardsToAllSubscribers) {
		// Arrange:
		DEFINE_MOCK_FLUSH_CAPTURE(UtChangeSubscriber);

		TestContext<MockUtChangeSubscriber> context;

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().flush();

		// Assert:
		test::AssertFlushDelegation(context);
	}
}}
