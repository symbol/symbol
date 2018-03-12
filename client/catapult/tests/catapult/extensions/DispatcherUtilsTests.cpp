#include "catapult/extensions/DispatcherUtils.h"
#include "catapult/config/NodeConfiguration.h"
#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/core/SchedulerTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/nodeps/Atomics.h"
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
		EXPECT_EQ(options.CacheDuration, 89 * 60 * 1000u);
		EXPECT_EQ(options.PruneInterval, 13 * 60 * 1000u);
		EXPECT_EQ(options.MaxCacheSize, 213455u);
	}

	TEST(TEST_CLASS, CanWrapTransactionStatusSubscriberInSink) {
		// Arrange:
		mocks::MockTransactionStatusSubscriber subscriber;
		auto transactionInfo = test::CreateRandomTransactionInfo();

		// Act:
		SubscriberToSink(subscriber)(
				*transactionInfo.pEntity,
				transactionInfo.EntityHash,
				static_cast<validators::ValidationResult>(97531u));

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
		auto pDispatcher = CreateDispatcher();
		pDispatcher->processElement(disruptor::ConsumerInput(test::CreateTransactionEntityRange(1)));
		pDispatcher->processElement(disruptor::ConsumerInput(test::CreateTransactionEntityRange(1)), [&](auto, const auto&) {
			isExecutingBlockedElementCallback.set();
			isElementCallbackUnblocked.wait();
		});
		pDispatcher->processElement(disruptor::ConsumerInput(test::CreateTransactionEntityRange(1)));

		// - wait until the blocked element callback is called
		isExecutingBlockedElementCallback.wait();

		// - create a locator and register the service
		auto keyPair = test::GenerateKeyPair();
		ServiceLocator locator(keyPair);
		locator.registerRootedService("foo", pDispatcher);

		// Act: register the counters
		AddDispatcherCounters(locator, "foo", "XYZ");
		std::unordered_map<std::string, size_t> counters;
		for (const auto& counter : locator.counters())
			counters[counter.id().name()] = counter.value();

		// Assert:
		ASSERT_EQ(2u, counters.size());
		EXPECT_EQ(3u, counters.at("XYZ ELEM TOT"));
		EXPECT_EQ(2u, counters.at("XYZ ELEM ACT"));

		// Cleanup:
		isElementCallbackUnblocked.set();
	}

	TEST(TEST_CLASS, CanCreateBatchTransactionTask) {
		// Arrange:
		auto pDispatcher = CreateDispatcher();
		TransactionBatchRangeDispatcher batchRangeDispatcher(*pDispatcher);

		// Act:
		auto task = CreateBatchTransactionTask(batchRangeDispatcher, "foo");

		// Assert:
		test::AssertUnscheduledTask(task, "batch foo task");
	}

	TEST(TEST_CLASS, BatchTransactionTaskDispatchesAllQueuedTransactions) {
		// Arrange:
		auto pDispatcher = CreateDispatcher();
		TransactionBatchRangeDispatcher batchRangeDispatcher(*pDispatcher);
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
