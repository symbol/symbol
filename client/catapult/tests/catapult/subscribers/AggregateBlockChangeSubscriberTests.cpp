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

#include "catapult/subscribers/AggregateBlockChangeSubscriber.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/catapult/subscribers/test/UnsupportedSubscribers.h"
#include "tests/test/core/BlockTestUtils.h"
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
		class MockBlockChangeSubscriber : public UnsupportedBlockChangeSubscriber {
		public:
			std::vector<const model::BlockElement*> Elements;

		public:
			void notifyBlock(const model::BlockElement& blockElement) override {
				Elements.push_back(&blockElement);
			}
		};

		TestContext<MockBlockChangeSubscriber> context;
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
			ASSERT_EQ(1u, pSubscriber->Elements.size()) << message;
			EXPECT_EQ(pBlockElement.get(), pSubscriber->Elements[0]) << message;
		}
	}

	TEST(TEST_CLASS, NotifyDropBlocksAfterForwardsToAllSubscribers) {
		// Arrange:
		class MockBlockChangeSubscriber : public UnsupportedBlockChangeSubscriber {
		public:
			std::vector<Height> Heights;

		public:
			void notifyDropBlocksAfter(Height height) override {
				Heights.push_back(height);
			}
		};

		TestContext<MockBlockChangeSubscriber> context;

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyDropBlocksAfter(Height(553));

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			ASSERT_EQ(1u, pSubscriber->Heights.size()) << message;
			EXPECT_EQ(Height(553), pSubscriber->Heights[0]) << message;
		}
	}
}}
