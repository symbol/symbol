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

#pragma once
#include "catapult/disruptor/DisruptorTypes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region ConsumerResult

	/// Asserts that \a result is continued.
	inline void AssertContinued(const disruptor::ConsumerResult& result) {
		EXPECT_EQ(disruptor::CompletionStatus::Normal, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
		EXPECT_EQ(disruptor::ConsumerResultSeverity::Success, result.ResultSeverity);
	}

	/// Asserts that \a result is aborted with \a code and \a resultSeverity.
	inline void AssertAborted(
			const disruptor::ConsumerResult& result,
			disruptor::CompletionCode code,
			disruptor::ConsumerResultSeverity resultSeverity) {
		auto message = "expected consumer code " + std::to_string(code);
		EXPECT_EQ(disruptor::CompletionStatus::Aborted, result.CompletionStatus) << message;
		EXPECT_EQ(code, result.CompletionCode) << message;
		EXPECT_EQ(resultSeverity, result.ResultSeverity) << message;
	}

	// endregion

	// region ConsumerCompletionResult

	/// Asserts that \a result is continued.
	inline void AssertContinued(const disruptor::ConsumerCompletionResult& result) {
		EXPECT_EQ(disruptor::CompletionStatus::Normal, result.CompletionStatus);
		EXPECT_EQ(0u, result.CompletionCode);
		EXPECT_EQ(disruptor::ConsumerResultSeverity::Success, result.ResultSeverity);
		EXPECT_EQ(std::numeric_limits<disruptor::PositionType>::max(), result.FinalConsumerPosition);
	}

	/// Asserts that \a result is aborted at consumer \a position with \a code and \a severity.
	inline void AssertAborted(
			const disruptor::ConsumerCompletionResult& result,
			disruptor::CompletionCode code,
			disruptor::ConsumerResultSeverity severity,
			disruptor::PositionType position) {
		auto message = "expected consumer at " + std::to_string(position);
		EXPECT_EQ(disruptor::CompletionStatus::Aborted, result.CompletionStatus) << message;
		EXPECT_EQ(code, result.CompletionCode) << message;
		EXPECT_EQ(severity, result.ResultSeverity) << message;
		EXPECT_EQ(position, result.FinalConsumerPosition) << message;
	}

	/// Asserts that \a expected and \a actual are equal.
	inline void AssertEqual(const disruptor::ConsumerCompletionResult& expected, const disruptor::ConsumerCompletionResult& actual) {
		auto message = "expected consumer at " + std::to_string(expected.FinalConsumerPosition);
		EXPECT_EQ(expected.CompletionStatus, actual.CompletionStatus) << message;
		EXPECT_EQ(expected.CompletionCode, actual.CompletionCode) << message;
		EXPECT_EQ(expected.ResultSeverity, actual.ResultSeverity) << message;
		EXPECT_EQ(expected.FinalConsumerPosition, actual.FinalConsumerPosition) << message;
	}

	// endregion
}}
