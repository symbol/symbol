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

#include "catapult/extensions/DispatcherUtils.h"
#include "catapult/config/CatapultKeys.h"
#include "catapult/config/NodeConfiguration.h"
#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/SchedulerTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/nodeps/Atomics.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/other/mocks/MockTransactionStatusSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS DispatcherUtilsTests

	TEST(TEST_CLASS, CanCreateHashCheckOptions) {
		// Arrange:
		auto nodeConfig = config::NodeConfiguration::Uninitialized();
		nodeConfig.ShortLivedCachePruneInterval = utils::TimeSpan::FromMinutes(13);
		nodeConfig.ShortLivedCacheMaxSize = 213455;

		// Act
		auto options = CreateHashCheckOptions(utils::TimeSpan::FromMinutes(89), nodeConfig);

		// Assert:
		EXPECT_EQ(89u * 60 * 1000, options.CacheDuration);
		EXPECT_EQ(13u * 60 * 1000, options.PruneInterval);
		EXPECT_EQ(213455u, options.MaxCacheSize);
	}

	TEST(TEST_CLASS, CanWrapTransactionStatusSubscriberInSink) {
		// Arrange:
		mocks::MockTransactionStatusSubscriber subscriber;
		auto transactionInfo = test::CreateRandomTransactionInfo();

		// Act:
		SubscriberToSink(subscriber)(
				*transactionInfo.pEntity,
				transactionInfo.EntityHash,
				static_cast<validators::ValidationResult>(97531));

		// Assert:
		ASSERT_EQ(1u, subscriber.params().size());

		const auto& capture = subscriber.params()[0];
		EXPECT_EQ(transactionInfo.pEntity.get(), &capture.Transaction);
		EXPECT_EQ(&transactionInfo.EntityHash, &capture.Hash);
		EXPECT_EQ(97531u, capture.Status);
	}

	namespace {
		auto CreateDispatcher() {
			auto options = disruptor::ConsumerDispatcherOptions{ "ConsumerDispatcherTests", 16u * 1024 };
			return std::make_shared<disruptor::ConsumerDispatcher>(options, std::vector<disruptor::DisruptorConsumer>{
				[](const auto&) { return disruptor::ConsumerResult::Continue(); }
			});
		}
	}

	TEST(TEST_CLASS, CanAddDispatcherCountersToLocator) {
		// Arrange: create a dispatcher with three elements and block the second element
		test::AutoSetFlag isExecutingBlockedElementCallback;
		test::AutoSetFlag isElementCallbackUnblocked;
		auto pIsExecuting = isExecutingBlockedElementCallback.state();
		auto pIsUnblocked = isElementCallbackUnblocked.state();

		auto pDispatcher = CreateDispatcher();
		auto input1 = disruptor::ConsumerInput(test::CreateTransactionEntityRange(1));
		auto input2 = disruptor::ConsumerInput(test::CreateTransactionEntityRange(1));
		auto input3 = disruptor::ConsumerInput(test::CreateTransactionEntityRange(1));
		pDispatcher->processElement(std::move(input1));
		pDispatcher->processElement(std::move(input2), [pIsExecuting, pIsUnblocked](auto, const auto&) {
			pIsExecuting->set();
			pIsUnblocked->wait();
		});
		pDispatcher->processElement(std::move(input3));

		// - wait until the blocked element callback is called
		isExecutingBlockedElementCallback.state()->wait();

		// - create a locator and register the service
		config::CatapultKeys keys;
		ServiceLocator locator(keys);
		locator.registerRootedService("foo", pDispatcher);

		// Act: register the counters
		AddDispatcherCounters(locator, "foo", "XYZ");
		std::unordered_map<std::string, size_t> counters;
		for (const auto& counter : locator.counters())
			counters[counter.id().name()] = counter.value();

		// Assert:
		ASSERT_EQ(3u, counters.size());
		EXPECT_EQ(3u, counters.at("XYZ ELEM TOT"));
		EXPECT_EQ(2u, counters.at("XYZ ELEM ACT"));
		EXPECT_EQ(0u, counters.at("XYZ ELEM MEM")); // total size is less than 1MB

		// Cleanup:
		isElementCallbackUnblocked.state()->set();
	}

	TEST(TEST_CLASS, CanCreateBatchTransactionTask) {
		// Arrange:
		auto pDispatcher = CreateDispatcher();
		TransactionBatchRangeDispatcher batchRangeDispatcher(*pDispatcher, model::NodeIdentityEqualityStrategy::Key_And_Host);

		// Act:
		auto task = CreateBatchTransactionTask(batchRangeDispatcher, "foo");

		// Assert:
		test::AssertUnscheduledTask(task, "batch foo task");
	}

	TEST(TEST_CLASS, BatchTransactionTaskDispatchesAllQueuedTransactions) {
		// Arrange:
		auto pDispatcher = CreateDispatcher();
		TransactionBatchRangeDispatcher batchRangeDispatcher(*pDispatcher, model::NodeIdentityEqualityStrategy::Key_And_Host);
		auto task = CreateBatchTransactionTask(batchRangeDispatcher, "foo");

		// - queue some transactions
		batchRangeDispatcher.queue(test::CreateTransactionEntityRange(1), disruptor::InputSource::Remote_Pull);
		batchRangeDispatcher.queue(test::CreateTransactionEntityRange(1), disruptor::InputSource::Remote_Pull);
		batchRangeDispatcher.queue(test::CreateTransactionEntityRange(1), disruptor::InputSource::Remote_Push);

		// Sanity: nothing was pushed to the dispatcher
		EXPECT_EQ(0u, pDispatcher->numAddedElements());

		// Act:
		task.Callback();

		// Assert: batched elements were pushed { (1, 2), (3) }
		EXPECT_EQ(2u, pDispatcher->numAddedElements());
	}
}}
