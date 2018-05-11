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

#include "catapult/subscribers/AggregateNodeSubscriber.h"
#include "catapult/ionet/Node.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/test/other/mocks/MockNodeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS AggregateNodeSubscriberTests

	namespace {
		template<typename TNodeSubscriber>
		using TestContext = test::AggregateSubscriberTestContext<TNodeSubscriber, AggregateNodeSubscriber<TNodeSubscriber>>;
	}

	TEST(TEST_CLASS, NotifyNodeForwardsToAllSubscribers) {
		// Arrange:
		TestContext<mocks::MockNodeSubscriber> context;
		auto node = ionet::Node();

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyNode(node);

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			const auto& capturedParams = pSubscriber->params();
			ASSERT_EQ(1u, capturedParams.size()) << message;
			EXPECT_EQ(&node, &capturedParams[0].Node) << message;
		}
	}
}}
