#include "catapult/subscribers/AggregateTransactionStatusSubscriber.h"
#include "catapult/model/TransactionStatus.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/other/mocks/MockTransactionStatusSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS AggregateTransactionStatusSubscriberTests

	namespace {
		template<typename TTransactionStatusSubscriber>
		using TestContext = test::AggregateSubscriberTestContext<
				TTransactionStatusSubscriber,
				AggregateTransactionStatusSubscriber<TTransactionStatusSubscriber>>;
	}

	TEST(TEST_CLASS, NotifyStatusForwardsToAllSubscribers) {
		// Arrange:
		TestContext<mocks::MockTransactionStatusSubscriber> context;
		auto pTransaction = test::GenerateRandomTransaction();
		auto hash = test::GenerateRandomData<Hash256_Size>();

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyStatus(*pTransaction, hash, 123);

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			const auto& capturedParams = pSubscriber->params();
			ASSERT_EQ(1u, capturedParams.size()) << message;
			EXPECT_EQ(pTransaction.get(), &capturedParams[0].Transaction) << message;
			EXPECT_EQ(hash, capturedParams[0].Hash) << message;
			EXPECT_EQ(123u, capturedParams[0].Status) << message;
		}
	}

	TEST(TEST_CLASS, FlushForwardsToAllSubscribers) {
		// Arrange:
		TestContext<mocks::MockTransactionStatusSubscriber> context;

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().flush();

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			ASSERT_EQ(1u, pSubscriber->numFlushes()) << message;
		}
	}
}}
