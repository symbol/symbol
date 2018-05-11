/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS BasicAggregateCosignaturesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(BasicAggregateCosignatures, 0, 0)

	namespace {
		auto GenerateRandomCosignatures(uint8_t numCosignatures) {
			return test::GenerateRandomDataVector<model::Cosignature>(numCosignatures);
		}
	}

	// region transactions count

	namespace {
		void AssertMaxTransactionsValidationResult(ValidationResult expectedResult, uint32_t numTransactions, uint32_t maxTransactions) {
			// Arrange: notice that transaction data is not actually checked
			auto signer = test::GenerateRandomData<Key_Size>();
			model::AggregateCosignaturesNotification notification(signer, numTransactions, nullptr, 0, nullptr);
			auto pValidator = CreateBasicAggregateCosignaturesValidator(maxTransactions, std::numeric_limits<uint8_t>::max());

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "txes " << numTransactions << ", max " << maxTransactions;
		}
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithZeroTransactions) {
		// Assert:
		AssertMaxTransactionsValidationResult(Failure_Aggregate_No_Transactions, 0, 100);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithLessThanMaxTransactions) {
		// Assert:
		AssertMaxTransactionsValidationResult(ValidationResult::Success, 1, 100);
		AssertMaxTransactionsValidationResult(ValidationResult::Success, 99, 100);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithExactlyMaxTransactions) {
		// Assert:
		AssertMaxTransactionsValidationResult(ValidationResult::Success, 100, 100);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithGreaterThanMaxTransactions) {
		// Assert:
		AssertMaxTransactionsValidationResult(Failure_Aggregate_Too_Many_Transactions, 101, 100);
		AssertMaxTransactionsValidationResult(Failure_Aggregate_Too_Many_Transactions, 999, 100);
	}

	// endregion

	// region cosignatures count

	namespace {
		void AssertMaxCosignaturesValidationResult(ValidationResult expectedResult, uint8_t numCosignatures, uint8_t maxCosignatures) {
			// Arrange:
			auto signer = test::GenerateRandomData<Key_Size>();
			auto cosignatures = GenerateRandomCosignatures(numCosignatures);
			model::AggregateCosignaturesNotification notification(signer, 3, nullptr, numCosignatures, cosignatures.data());
			auto pValidator = CreateBasicAggregateCosignaturesValidator(std::numeric_limits<uint32_t>::max(), maxCosignatures);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "cosignatures " << static_cast<uint16_t>(numCosignatures)
					<< ", max " << static_cast<uint16_t>(maxCosignatures);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithZeroExplicitCosignatures) {
		// Assert: notice that there is always one implicit cosigner (the tx signer)
		AssertMaxCosignaturesValidationResult(ValidationResult::Success, 0, 100);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithLessThanMaxCosignatures) {
		// Assert:
		AssertMaxCosignaturesValidationResult(ValidationResult::Success, 1, 100);
		AssertMaxCosignaturesValidationResult(ValidationResult::Success, 98, 100);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithExactlyMaxCosignatures) {
		// Assert:
		AssertMaxCosignaturesValidationResult(ValidationResult::Success, 99, 100);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithGreaterThanMaxCosignatures) {
		// Assert:
		AssertMaxCosignaturesValidationResult(Failure_Aggregate_Too_Many_Cosignatures, 100, 100);
		AssertMaxCosignaturesValidationResult(Failure_Aggregate_Too_Many_Cosignatures, 222, 100);
	}

	// endregion

	// region cosigner uniqueness

	namespace {
		void AssertCosignerUniquenessValidationResult(
				ValidationResult expectedResult,
				const Key& signer,
				const std::vector<model::Cosignature>& cosignatures) {
			// Arrange:
			model::AggregateCosignaturesNotification notification(signer, 3, nullptr, cosignatures.size(), cosignatures.data());
			auto pValidator = CreateBasicAggregateCosignaturesValidator(
					std::numeric_limits<uint32_t>::max(),
					std::numeric_limits<uint8_t>::max());

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithAllCosignersBeingUnique) {
		// Arrange: no conflicts
		auto signer = test::GenerateRandomData<Key_Size>();
		auto cosignatures = GenerateRandomCosignatures(5);

		// Assert:
		AssertCosignerUniquenessValidationResult(ValidationResult::Success, signer, cosignatures);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithRedundantExplicitAndImplicitCosigner) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto cosignatures = GenerateRandomCosignatures(5);
		cosignatures[2].Signer = signer;

		// Assert:
		AssertCosignerUniquenessValidationResult(Failure_Aggregate_Redundant_Cosignatures, signer, cosignatures);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithRedundantImplicitCosigners) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto cosignatures = GenerateRandomCosignatures(5);
		cosignatures[0].Signer = cosignatures[4].Signer;

		// Assert:
		AssertCosignerUniquenessValidationResult(Failure_Aggregate_Redundant_Cosignatures, signer, cosignatures);
	}

	// endregion
}}
