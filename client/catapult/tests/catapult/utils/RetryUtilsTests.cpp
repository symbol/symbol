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

#include "catapult/utils/RetryUtils.h"
#include "tests/TestHarness.h"
#include <system_error>
#include <vector>

namespace catapult { namespace utils {

#define TEST_CLASS RetryUtilsTests

	namespace {
		auto Success() {
			return std::error_code();
		}

		auto RetryableError() {
			return std::make_error_code(std::errc::permission_denied);
		}

		auto NonRetryableError() {
			return std::make_error_code(std::errc::no_such_file_or_directory);
		}

		auto IsRetryable(const std::error_code& ec) {
			return std::errc::permission_denied == ec;
		}

		struct RetryCall {
		public:
			uint32_t Attempt;
			std::error_code Error;
			uint32_t DelayMs;
		};

		// Tracks operation invocations and onRetry (attempt, error, delay) callbacks for assertions.
		struct RetryContext {
		public:
			size_t NumOperationCalls = 0;
			std::vector<RetryCall> RetryCalls;

		public:
			auto onRetry() {
				return [this](auto attempt, const auto& ec, auto delayMs) {
					RetryCalls.push_back({ attempt, ec, delayMs });
				};
			}
		};

		// Creates an operation that increments \a context's call counter and always returns \a result.
		auto MakeOperation(RetryContext& context, std::error_code result) {
			return [&context, result]() {
				++context.NumOperationCalls;
				return result;
			};
		}
	}

	TEST(TEST_CLASS, SucceedsOnFirstAttempt_WithoutRetrying) {
		// Arrange:
		RetryContext context;
		auto operation = MakeOperation(context, Success());

		// Act:
		auto result = RetryWithBackoff(operation, IsRetryable, 5u, context.onRetry(), 0);

		// Assert:
		EXPECT_EQ(std::error_code(), result);
		EXPECT_EQ(1u, context.NumOperationCalls);
		EXPECT_TRUE(context.RetryCalls.empty());
	}

	TEST(TEST_CLASS, StopsImmediately_WhenErrorIsNotRetryable) {
		// Arrange:
		RetryContext context;
		auto operation = MakeOperation(context, NonRetryableError());

		// Act:
		auto result = RetryWithBackoff(operation, IsRetryable, 5u, context.onRetry(), 0);

		// Assert:
		EXPECT_EQ(NonRetryableError(), result);
		EXPECT_EQ(1u, context.NumOperationCalls);
		EXPECT_TRUE(context.RetryCalls.empty());
	}

	TEST(TEST_CLASS, RetriesUntilSuccess_WhenErrorIsRetryable) {
		// Arrange: fail twice, then succeed
		RetryContext context;
		auto operation = [&context]() {
			++context.NumOperationCalls;
			return 3 == context.NumOperationCalls ? Success() : RetryableError();
		};

		// Act:
		auto result = RetryWithBackoff(operation, IsRetryable, 5u, context.onRetry(), 0);

		// Assert:
		EXPECT_EQ(std::error_code(), result);
		EXPECT_EQ(3u, context.NumOperationCalls);
		ASSERT_EQ(2u, context.RetryCalls.size());
		EXPECT_EQ(0u, context.RetryCalls[0].Attempt);
		EXPECT_EQ(1u, context.RetryCalls[1].Attempt);
		EXPECT_EQ(RetryableError(), context.RetryCalls[0].Error);
		EXPECT_EQ(RetryableError(), context.RetryCalls[1].Error);
	}

	TEST(TEST_CLASS, GivesUpAfterExhaustingAttempts_WhenErrorPersists) {
		// Arrange:
		RetryContext context;
		auto operation = MakeOperation(context, RetryableError());

		// Act:
		auto result = RetryWithBackoff(operation, IsRetryable, 3u, context.onRetry(), 0);

		// Assert: three attempts total, two retries in between
		EXPECT_EQ(RetryableError(), result);
		EXPECT_EQ(3u, context.NumOperationCalls);
		EXPECT_EQ(2u, context.RetryCalls.size());
	}

	TEST(TEST_CLASS, NeverRetries_WhenNumAttemptsIsOne) {
		// Arrange:
		RetryContext context;
		auto operation = MakeOperation(context, RetryableError());

		// Act:
		auto result = RetryWithBackoff(operation, IsRetryable, 1u, context.onRetry(), 0);

		// Assert:
		EXPECT_EQ(RetryableError(), result);
		EXPECT_EQ(1u, context.NumOperationCalls);
		EXPECT_TRUE(context.RetryCalls.empty());
	}

	TEST(TEST_CLASS, DelayDoublesEachAttempt_WhenBaseDelayIsNonzero) {
		// Arrange:
		RetryContext context;
		auto operation = MakeOperation(context, RetryableError());

		// Act: use a tiny base delay so the (real) sleeps stay negligible
		auto result = RetryWithBackoff(operation, IsRetryable, 4u, context.onRetry(), 1);

		// Assert:
		EXPECT_EQ(RetryableError(), result);
		ASSERT_EQ(3u, context.RetryCalls.size());
		EXPECT_EQ(1u, context.RetryCalls[0].DelayMs);
		EXPECT_EQ(2u, context.RetryCalls[1].DelayMs);
		EXPECT_EQ(4u, context.RetryCalls[2].DelayMs);
	}

	TEST(TEST_CLASS, ZeroBaseDelaySkipsWait) {
		// Arrange:
		RetryContext context;
		auto operation = MakeOperation(context, RetryableError());

		// Act:
		RetryWithBackoff(operation, IsRetryable, 3u, context.onRetry(), 0);

		// Assert: delay is reported as zero to onRetry (and, per contract, no sleep is performed)
		ASSERT_EQ(2u, context.RetryCalls.size());
		EXPECT_EQ(0u, context.RetryCalls[0].DelayMs);
		EXPECT_EQ(0u, context.RetryCalls[1].DelayMs);
	}
}}
