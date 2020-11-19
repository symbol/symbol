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

#include "mongo/src/MongoErrorPolicy.h"
#include "mongo/src/MongoBulkWriter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS MongoErrorPolicyTests

	// region constructor / mode

	TEST(TEST_CLASS, CanCreateStrictErrorPolicy) {
		// Act:
		MongoErrorPolicy errorPolicy("foo", MongoErrorPolicy::Mode::Strict);

		// Assert:
		EXPECT_EQ(MongoErrorPolicy::Mode::Strict, errorPolicy.mode());
	}

	TEST(TEST_CLASS, CanCreateIdempotentErrorPolicy) {
		// Act:
		MongoErrorPolicy errorPolicy("foo", MongoErrorPolicy::Mode::Idempotent);

		// Assert:
		EXPECT_EQ(MongoErrorPolicy::Mode::Idempotent, errorPolicy.mode());
	}

	// endregion

	// region equal constraint - traits

	namespace {
		struct DeletedTraits {
			static constexpr auto CheckerFunc = &MongoErrorPolicy::checkDeleted;

			static void SetValue(BulkWriteResult& result, int32_t value) {
				result.NumDeleted = value;
			}
		};

		struct InsertedTraits {
			static constexpr auto CheckerFunc = &MongoErrorPolicy::checkInserted;

			static void SetValue(BulkWriteResult& result, int32_t value) {
				result.NumInserted = value;
			}
		};

		struct UpsertedTraits {
			static constexpr auto CheckerFunc = &MongoErrorPolicy::checkUpserted;

			static void SetValue(BulkWriteResult& result, int32_t value) {
				result.NumUpserted = value / 25;
				result.NumModified = value - result.NumUpserted;
			}
		};
	}

#define EQUAL_CONSTRAINT_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Deleted) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeletedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Inserted) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<InsertedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Upserted) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UpsertedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region strict mode - equal constraint - tests

	EQUAL_CONSTRAINT_TEST(StrictCheckThrowsWhenExpectedIsNotEqualToActual) {
		// Arrange:
		MongoErrorPolicy errorPolicy("foo", MongoErrorPolicy::Mode::Strict);
		auto checkerFunc = TTraits::CheckerFunc;

		BulkWriteResult result;
		TTraits::SetValue(result, 101);

		// Act + Assert:
		for (auto numExpected : { 0u, 1u, 50u, 100u, 102u, 150u, 999u })
			EXPECT_THROW((errorPolicy.*checkerFunc)(numExpected, result, "rabbits"), catapult_runtime_error) << numExpected;
	}

	EQUAL_CONSTRAINT_TEST(StrictCheckDoesNotThrowWhenExpectedIsEqualToActual) {
		// Arrange:
		MongoErrorPolicy errorPolicy("foo", MongoErrorPolicy::Mode::Strict);
		auto checkerFunc = TTraits::CheckerFunc;

		BulkWriteResult result;
		TTraits::SetValue(result, 101);

		// Act + Assert:
		EXPECT_NO_THROW((errorPolicy.*checkerFunc)(101, result, "rabbits"));
	}

	// endregion

	// region at least constraint - traits

	namespace {
		struct DeletedAtLeastTraits : public DeletedTraits {
			static constexpr auto CheckerFunc = &MongoErrorPolicy::checkDeletedAtLeast;
		};
	}

#define AT_LEAST_CONSTRAINT_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Deleted) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeletedAtLeastTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region strict mode - at least constraint - tests

	AT_LEAST_CONSTRAINT_TEST(StrictAtLeastCheckThrowsWhenExpectedIsGreaterThanActual) {
		// Arrange:
		MongoErrorPolicy errorPolicy("foo", MongoErrorPolicy::Mode::Strict);
		auto checkerFunc = TTraits::CheckerFunc;

		BulkWriteResult result;
		TTraits::SetValue(result, 101);

		// Act + Assert:
		for (auto numExpected : { 102u, 150u, 999u })
			EXPECT_THROW((errorPolicy.*checkerFunc)(numExpected, result, "rabbits"), catapult_runtime_error) << numExpected;
	}

	AT_LEAST_CONSTRAINT_TEST(StrictAtLeastCheckDoesNotThrowWhenExpectedIsLessThanOrEqualToActual) {
		// Arrange:
		MongoErrorPolicy errorPolicy("foo", MongoErrorPolicy::Mode::Strict);
		auto checkerFunc = TTraits::CheckerFunc;

		BulkWriteResult result;
		TTraits::SetValue(result, 101);

		// Act + Assert:
		for (auto numExpected : { 0u, 1u, 50u, 100u, 101u })
			EXPECT_NO_THROW((errorPolicy.*checkerFunc)(numExpected, result, "rabbits")) << numExpected;
	}

	// endregion

	// region idempotent mode - equal constraint - tests

	EQUAL_CONSTRAINT_TEST(IdempotentCheckDoesNotThrowWhenExpectedIsGreaterThanActual) {
		// Arrange:
		MongoErrorPolicy errorPolicy("foo", MongoErrorPolicy::Mode::Idempotent);
		auto checkerFunc = TTraits::CheckerFunc;

		BulkWriteResult result;
		TTraits::SetValue(result, 101);

		// Act + Assert:
		for (auto numExpected : { 102u, 150u, 999u })
			EXPECT_NO_THROW((errorPolicy.*checkerFunc)(numExpected, result, "rabbits")) << numExpected;
	}

	EQUAL_CONSTRAINT_TEST(IdempotentCheckDoesNotThrowWhenExpectedIsEqualToActual) {
		// Arrange:
		MongoErrorPolicy errorPolicy("foo", MongoErrorPolicy::Mode::Idempotent);
		auto checkerFunc = TTraits::CheckerFunc;

		BulkWriteResult result;
		TTraits::SetValue(result, 101);

		// Act + Assert:
		EXPECT_NO_THROW((errorPolicy.*checkerFunc)(101, result, "rabbits"));
	}

	EQUAL_CONSTRAINT_TEST(IdempotentCheckThrowsWhenExpectedIsLessThanActual) {
		// Arrange:
		MongoErrorPolicy errorPolicy("foo", MongoErrorPolicy::Mode::Idempotent);
		auto checkerFunc = TTraits::CheckerFunc;

		BulkWriteResult result;
		TTraits::SetValue(result, 101);

		// Act + Assert:
		for (auto numExpected : { 0u, 1u, 50u, 100u })
			EXPECT_THROW((errorPolicy.*checkerFunc)(numExpected, result, "rabbits"), catapult_runtime_error) << numExpected;
	}

	// endregion

	// region idempotent mode - at least constraint - tests

	AT_LEAST_CONSTRAINT_TEST(IdempotentAtLeastCheckNeverThrows) {
		// Arrange:
		MongoErrorPolicy errorPolicy("foo", MongoErrorPolicy::Mode::Idempotent);
		auto checkerFunc = TTraits::CheckerFunc;

		BulkWriteResult result;
		TTraits::SetValue(result, 101);

		// Act + Assert:
		for (auto numExpected : { 0u, 1u, 50u, 100u, 101u, 102u, 150u, 999u })
			EXPECT_NO_THROW((errorPolicy.*checkerFunc)(numExpected, result, "rabbits")) << numExpected;
	}

	// endregion
}}
