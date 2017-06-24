#pragma once
#include "DispatcherTestUtils.h"
#include "catapult/local/LocalNodeStats.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region basic

	/// Asserts that the dispatcher service can be booted with the expected number of dispatcher consumers
	/// (\a numExpectedBlockConsumers and \a numExpectedTransactionConsumers).
	template<typename TTestContext>
	void AssertCanBootDispatcherService(size_t numExpectedBlockConsumers, size_t numExpectedTransactionConsumers) {
		// Arrange:
		TTestContext context;
		auto& service = context.service();

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(0u, service.numAddedBlockElements());
		EXPECT_EQ(0u, service.numAddedTransactionElements());

		auto blockDispatcherStatus = service.blockDispatcherStatus();
		EXPECT_EQ("block dispatcher", blockDispatcherStatus.Name);
		EXPECT_EQ(numExpectedBlockConsumers, blockDispatcherStatus.Size);
		EXPECT_TRUE(blockDispatcherStatus.IsRunning);

		auto transactionDispatcherStatus = service.transactionDispatcherStatus();
		EXPECT_EQ("transaction dispatcher", transactionDispatcherStatus.Name);
		EXPECT_EQ(numExpectedTransactionConsumers, transactionDispatcherStatus.Size);
		EXPECT_TRUE(transactionDispatcherStatus.IsRunning);
	}

	/// Asserts that the dispatcher service can be shutdown.
	template<typename TTestContext>
	void AssertCanShutdownDispatcherService() {
		// Arrange:
		TTestContext context;
		auto& service = context.service();
		context.boot();

		// Act:
		context.pool().shutdown();

		// Assert:
		EXPECT_EQ(local::Sentinel_Stats_Value, service.numAddedBlockElements());
		EXPECT_EQ(local::Sentinel_Stats_Value, service.numAddedTransactionElements());

		auto blockDispatcherStatus = service.blockDispatcherStatus();
		EXPECT_EQ("", blockDispatcherStatus.Name);
		EXPECT_EQ(0u, blockDispatcherStatus.Size);
		EXPECT_FALSE(blockDispatcherStatus.IsRunning);

		auto transactionDispatcherStatus = service.transactionDispatcherStatus();
		EXPECT_EQ("", transactionDispatcherStatus.Name);
		EXPECT_EQ(0u, transactionDispatcherStatus.Size);
		EXPECT_FALSE(transactionDispatcherStatus.IsRunning);
	}

	// endregion

	// region consume

	/// Asserts that the dispatcher service can consume \a range and then calls \a handler.
	template<typename TTestContext, typename THandler>
	void AssertCanConsumeBlockRange(model::BlockRange&& range, THandler handler) {
		// Arrange:
		TTestContext context;
		auto& service = context.service();
		auto factory = service.createBlockRangeConsumerFactory()(disruptor::InputSource::Local);
		context.boot();

		// Act:
		factory(std::move(range));
		WAIT_FOR_ONE_EXPR(service.numAddedBlockElements());

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Assert:
		EXPECT_EQ(1u, service.numAddedBlockElements());
		EXPECT_EQ(0u, service.numAddedTransactionElements());
		handler(context);
	}

	/// Asserts that the dispatcher service can consume \a range and then calls \a handler.
	template<typename TTestContext, typename THandler>
	void AssertCanConsumeBlockRangeCompletionAware(model::BlockRange&& range, THandler handler) {
		// Arrange:
		TTestContext context;
		auto& service = context.service();
		auto factory = service.createCompletionAwareBlockRangeConsumerFactory()(disruptor::InputSource::Local);
		context.boot();

		// Act:
		std::atomic<size_t> numCompletions(0);
		factory(std::move(range), [&numCompletions](auto, auto) { ++numCompletions; });
		WAIT_FOR_ONE(numCompletions);

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Assert:
		EXPECT_EQ(1u, numCompletions);
		EXPECT_EQ(1u, service.numAddedBlockElements());
		EXPECT_EQ(0u, service.numAddedTransactionElements());
		handler(context);
	}

	/// Asserts that the dispatcher service can consume \a range and then calls \a handler.
	template<typename TTestContext, typename THandler>
	void AssertCanConsumeTransactionRange(model::TransactionRange&& range, THandler handler) {
		// Arrange:
		TTestContext context;
		auto& service = context.service();
		auto factory = service.createTransactionRangeConsumerFactory()(disruptor::InputSource::Local);
		context.boot();

		// Act:
		factory(std::move(range));
		WAIT_FOR_ONE_EXPR(service.numAddedTransactionElements());

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Assert:
		EXPECT_EQ(0u, service.numAddedBlockElements());
		EXPECT_EQ(1u, service.numAddedTransactionElements());
		handler(context);
	}

	// endregion
}}
