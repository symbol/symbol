#include "catapult/consumers/ConsumerResultFactory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace consumers {

	TEST(ConsumerResultFactoryTests, CanCreateContinueConsumerResult) {
		// Act:
		auto result = Continue();

		// Assert:
		EXPECT_EQ(disruptor::CompletionStatus::Normal, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
	}

	TEST(ConsumerResultFactoryTests, CanCreateAbortConsumerResult) {
		// Act:
		auto result = Abort(static_cast<validators::ValidationResult>(456));

		// Assert:
		EXPECT_EQ(disruptor::CompletionStatus::Aborted, result.CompletionStatus);
		EXPECT_EQ(456u, result.CompletionCode);
	}

	TEST(ConsumerResultFactoryTests, CanCreateCompleteConsumerResult) {
		// Act:
		auto result = Complete();

		// Assert:
		EXPECT_EQ(disruptor::CompletionStatus::Consumed, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
	}
}}
