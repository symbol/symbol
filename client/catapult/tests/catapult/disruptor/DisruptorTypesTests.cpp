#include "catapult/disruptor/DisruptorTypes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace disruptor {

#define TEST_CLASS DisruptorTypesTests

	// region ConsumerResult

	TEST(TEST_CLASS, CanCreateDefaultConsumerResult) {
		// Act:
		auto result = ConsumerResult();

		// Assert:
		EXPECT_EQ(CompletionStatus::Normal, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
	}

	TEST(TEST_CLASS, CanCreateContinueConsumerResult) {
		// Act:
		auto result = ConsumerResult::Continue();

		// Assert:
		EXPECT_EQ(CompletionStatus::Normal, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
	}

	TEST(TEST_CLASS, CanCreateAbortConsumerResult) {
		// Act:
		auto result = ConsumerResult::Abort();

		// Assert:
		EXPECT_EQ(CompletionStatus::Aborted, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
	}

	TEST(TEST_CLASS, CanCreateAbortConsumerResultWithCustomCompletionCode) {
		// Act:
		auto result = ConsumerResult::Abort(456);

		// Assert:
		EXPECT_EQ(CompletionStatus::Aborted, result.CompletionStatus);
		EXPECT_EQ(456u, result.CompletionCode);
	}

	TEST(TEST_CLASS, CanCreateCompleteConsumerResult) {
		// Act:
		auto result = ConsumerResult::Complete();

		// Assert:
		EXPECT_EQ(CompletionStatus::Consumed, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
	}

	// endregion

	// region ConsumerCompletionResult

	TEST(TEST_CLASS, CanCreateDefaultConsumerCompletionResult) {
		// Act:
		auto result = ConsumerCompletionResult();

		// Assert:
		EXPECT_EQ(CompletionStatus::Normal, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
		EXPECT_EQ(std::numeric_limits<uint64_t>::max(), result.FinalConsumerPosition);
	}

	// endregion
}}
