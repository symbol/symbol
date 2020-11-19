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

#include "catapult/validators/AggregateValidationResult.h"
#include "tests/test/other/ValidationResultTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AggregateValidationResultTests

	namespace {
		constexpr auto Success_Result = ValidationResult::Success;
		constexpr auto Success2_Result = test::MakeValidationResult(ResultSeverity::Success, 1);
		constexpr auto Neutral_Result = test::MakeValidationResult(ResultSeverity::Neutral, 2);
		constexpr auto Failure_Result = test::MakeValidationResult(ResultSeverity::Failure, 3);
		constexpr auto Failure2_Result = test::MakeValidationResult(ResultSeverity::Failure, 4);

		template<typename TAggregate>
		ValidationResult AggregateAndReturn(TAggregate& aggregate, ValidationResult value) {
			AggregateValidationResult(aggregate, value);
			return aggregate;
		}

		struct AtomicResultTraits {
			static ValidationResult AggregateTwo(ValidationResult seed, ValidationResult value) {
				std::atomic<ValidationResult> aggregate(seed);
				return AggregateAndReturn(aggregate, value);
			}
		};

		struct RawResultTraits {
			static ValidationResult AggregateTwo(ValidationResult seed, ValidationResult value) {
				ValidationResult aggregate = seed;
				return AggregateAndReturn(aggregate, value);
			}
		};
	}

#define AGGREGATE_RESULT_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Atomic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AtomicResultTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Raw) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RawResultTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	AGGREGATE_RESULT_TRAITS_BASED_TEST(CanAggregateFromTrueSuccess) {
		EXPECT_EQ(Success_Result, TTraits::AggregateTwo(Success_Result, Success_Result));
		EXPECT_EQ(Success_Result, TTraits::AggregateTwo(Success_Result, Success2_Result));
		EXPECT_EQ(Neutral_Result, TTraits::AggregateTwo(Success_Result, Neutral_Result));
		EXPECT_EQ(Failure_Result, TTraits::AggregateTwo(Success_Result, Failure_Result));
	}

	AGGREGATE_RESULT_TRAITS_BASED_TEST(CanAggregateFromOtherSuccess) {
		EXPECT_EQ(Success2_Result, TTraits::AggregateTwo(Success2_Result, Success_Result));
		EXPECT_EQ(Success2_Result, TTraits::AggregateTwo(Success2_Result, Success2_Result));
		EXPECT_EQ(Neutral_Result, TTraits::AggregateTwo(Success2_Result, Neutral_Result));
		EXPECT_EQ(Failure_Result, TTraits::AggregateTwo(Success2_Result, Failure_Result));
	}

	AGGREGATE_RESULT_TRAITS_BASED_TEST(CanAggregateFromNeutral) {
		EXPECT_EQ(Neutral_Result, TTraits::AggregateTwo(Neutral_Result, Success_Result));
		EXPECT_EQ(Neutral_Result, TTraits::AggregateTwo(Neutral_Result, Success2_Result));
		EXPECT_EQ(Neutral_Result, TTraits::AggregateTwo(Neutral_Result, Neutral_Result));
		EXPECT_EQ(Failure_Result, TTraits::AggregateTwo(Neutral_Result, Failure_Result));
	}

	AGGREGATE_RESULT_TRAITS_BASED_TEST(CanAggregateFromFailure) {
		EXPECT_EQ(Failure_Result, TTraits::AggregateTwo(Failure_Result, Success_Result));
		EXPECT_EQ(Failure_Result, TTraits::AggregateTwo(Failure_Result, Success2_Result));
		EXPECT_EQ(Failure_Result, TTraits::AggregateTwo(Failure_Result, Neutral_Result));
		EXPECT_EQ(Failure_Result, TTraits::AggregateTwo(Failure_Result, Failure_Result));
		EXPECT_EQ(Failure_Result, TTraits::AggregateTwo(Failure_Result, Failure2_Result)); // FIRST failure has highest precedence
	}
}}
