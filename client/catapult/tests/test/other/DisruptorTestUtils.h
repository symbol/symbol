#pragma once
#include "catapult/disruptor/DisruptorTypes.h"
#include "catapult/preprocessor.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region ConsumerResult

	/// Asserts that \a result is continued.
	CATAPULT_INLINE
	void AssertContinued(const disruptor::ConsumerResult& result) {
		EXPECT_EQ(disruptor::CompletionStatus::Normal, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
	}

	/// Asserts that \a result is aborted with \a code.
	CATAPULT_INLINE
	void AssertAborted(const disruptor::ConsumerResult& result, disruptor::CompletionCode code) {
		auto message = "expected consumer code " + std::to_string(code);
		EXPECT_EQ(disruptor::CompletionStatus::Aborted, result.CompletionStatus) << message;
		EXPECT_EQ(code, result.CompletionCode) << message;
	}

	// endregion

	// region ConsumerCompletionResult

	/// Asserts that \a result is continued.
	CATAPULT_INLINE
	void AssertContinued(const disruptor::ConsumerCompletionResult& result) {
		EXPECT_EQ(disruptor::CompletionStatus::Normal, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
		EXPECT_EQ(std::numeric_limits<disruptor::PositionType>::max(), result.FinalConsumerPosition);
	}

	/// Asserts that \a result is aborted at consumer \a position with \a code.
	CATAPULT_INLINE
	void AssertAborted(
			const disruptor::ConsumerCompletionResult& result,
			disruptor::CompletionCode code,
			disruptor::PositionType position) {
		auto message = "expected consumer at " + std::to_string(position);
		EXPECT_EQ(disruptor::CompletionStatus::Aborted, result.CompletionStatus) << message;
		EXPECT_EQ(code, result.CompletionCode) << message;
		EXPECT_EQ(position, result.FinalConsumerPosition) << message;
	}

	/// Asserts that \a expected and \a actual are equal.
	CATAPULT_INLINE
	void AssertEqual(const disruptor::ConsumerCompletionResult& expected, const disruptor::ConsumerCompletionResult& actual) {
		auto message = "expected consumer at " + std::to_string(expected.FinalConsumerPosition);
		EXPECT_EQ(expected.CompletionStatus, actual.CompletionStatus) << message;
		EXPECT_EQ(expected.CompletionCode, actual.CompletionCode) << message;
		EXPECT_EQ(expected.FinalConsumerPosition, actual.FinalConsumerPosition) << message;
	}

	// endregion
}}
