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
			const auto& capturedParams = pSubscriber->nodeParams().params();
			ASSERT_EQ(1u, capturedParams.size()) << message;
			EXPECT_EQ(&node, &capturedParams[0].Node) << message;
		}
	}

	namespace {
		template<typename TPrepare>
		void AssertNotifyIncomingNodeForwardsToSubscribers(bool expectedResult, size_t maxSubscriberForwardIndex, TPrepare prepare) {
			// Arrange:
			TestContext<mocks::MockNodeSubscriber> context;
			prepare(context.subscribers());
			auto key = test::GenerateRandomByteArray<Key>();

			// Sanity:
			EXPECT_EQ(3u, context.subscribers().size());

			// Act:
			auto result = context.aggregate().notifyIncomingNode({ key, "11.22.33.44" }, ionet::ServiceIdentifier(212));

			// Assert:
			EXPECT_EQ(expectedResult, result);

			auto i = 0u;
			for (const auto* pSubscriber : context.subscribers()) {
				auto message = "subscriber at " + std::to_string(i);
				const auto& capturedParams = pSubscriber->incomingNodeParams().params();
				if (i <= maxSubscriberForwardIndex) {
					ASSERT_EQ(1u, capturedParams.size()) << message;
					EXPECT_EQ(key, capturedParams[0].Identity.PublicKey) << message;
					EXPECT_EQ("11.22.33.44", capturedParams[0].Identity.Host) << message;
					EXPECT_EQ(ionet::ServiceIdentifier(212), capturedParams[0].ServiceId) << message;
				} else {
					EXPECT_EQ(0u, capturedParams.size()) << message;
				}

				++i;
			}
		}
	}

	TEST(TEST_CLASS, NotifyIncomingNodeForwardsToAllSubscribers_TrueResult) {
		AssertNotifyIncomingNodeForwardsToSubscribers(true, 2, [](const auto&) {});
	}

	TEST(TEST_CLASS, NotifyIncomingNodeForwardsToShortCircuitSubscribers_FalseResult) {
		AssertNotifyIncomingNodeForwardsToSubscribers(false, 1, [](auto& subscribers) {
			subscribers[1]->setNotifyIncomingNodeResult(false);
		});
	}

	TEST(TEST_CLASS, NotifyBanForwardsToAllSubscribers) {
		// Arrange:
		TestContext<mocks::MockNodeSubscriber> context;
		auto key = test::GenerateRandomByteArray<Key>();

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyBan({ key, "11.22.33.44" }, 123);

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			const auto& capturedParams = pSubscriber->banParams().params();
			ASSERT_EQ(1u, capturedParams.size()) << message;
			EXPECT_EQ(key, capturedParams[0].Identity.PublicKey) << message;
			EXPECT_EQ("11.22.33.44", capturedParams[0].Identity.Host) << message;
			EXPECT_EQ(123u, capturedParams[0].Reason) << message;
		}
	}
}}
