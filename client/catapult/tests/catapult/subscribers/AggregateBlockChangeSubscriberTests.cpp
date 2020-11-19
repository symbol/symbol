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

#include "catapult/subscribers/AggregateBlockChangeSubscriber.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/catapult/subscribers/test/UnsupportedSubscribers.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/other/mocks/MockBlockChangeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS AggregateBlockChangeSubscriberTests

	namespace {
		using UnsupportedBlockChangeSubscriber = test::UnsupportedBlockChangeSubscriber;

		template<typename TBlockChangeSubscriber>
		using TestContext = test::AggregateSubscriberTestContext<
				TBlockChangeSubscriber,
				AggregateBlockChangeSubscriber<TBlockChangeSubscriber>>;
	}

	TEST(TEST_CLASS, NotifyBlockForwardsToAllSubscribers) {
		// Arrange:
		TestContext<mocks::MockBlockChangeSubscriber> context;
		auto pBlock = test::GenerateEmptyRandomBlock();
		auto pBlockElement = std::make_shared<model::BlockElement>(*pBlock);

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyBlock(*pBlockElement);

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			ASSERT_EQ(1u, pSubscriber->blockElements().size()) << message;
			EXPECT_EQ(pBlockElement.get(), pSubscriber->blockElements()[0]) << message;
		}
	}

	TEST(TEST_CLASS, NotifyDropBlocksAfterForwardsToAllSubscribers) {
		// Arrange:
		TestContext<mocks::MockBlockChangeSubscriber> context;

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyDropBlocksAfter(Height(553));

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			ASSERT_EQ(1u, pSubscriber->dropBlocksAfterHeights().size()) << message;
			EXPECT_EQ(Height(553), pSubscriber->dropBlocksAfterHeights()[0]) << message;
		}
	}
}}
