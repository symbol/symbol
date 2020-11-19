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
			auto signer = test::GenerateRandomByteArray<Key>();
			model::AggregateCosignaturesNotification notification(signer, numTransactions, nullptr, 0, nullptr);
			auto pValidator = CreateBasicAggregateCosignaturesValidator(maxTransactions, std::numeric_limits<uint8_t>::max());

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "txes " << numTransactions << ", max " << maxTransactions;
		}
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithZeroTransactions) {
		AssertMaxTransactionsValidationResult(Failure_Aggregate_No_Transactions, 0, 100);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithLessThanMaxTransactions) {
		AssertMaxTransactionsValidationResult(ValidationResult::Success, 1, 100);
		AssertMaxTransactionsValidationResult(ValidationResult::Success, 99, 100);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithExactlyMaxTransactions) {
		AssertMaxTransactionsValidationResult(ValidationResult::Success, 100, 100);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithGreaterThanMaxTransactions) {
		AssertMaxTransactionsValidationResult(Failure_Aggregate_Too_Many_Transactions, 101, 100);
		AssertMaxTransactionsValidationResult(Failure_Aggregate_Too_Many_Transactions, 999, 100);
	}

	// endregion

	// region cosignatures count

	namespace {
		void AssertMaxCosignaturesValidationResult(ValidationResult expectedResult, uint8_t numCosignatures, uint8_t maxCosignatures) {
			// Arrange:
			auto signer = test::GenerateRandomByteArray<Key>();
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
		// Assert: notice that there is always one implicit cosignatory (the tx signer)
		AssertMaxCosignaturesValidationResult(ValidationResult::Success, 0, 100);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithLessThanMaxCosignatures) {
		AssertMaxCosignaturesValidationResult(ValidationResult::Success, 1, 100);
		AssertMaxCosignaturesValidationResult(ValidationResult::Success, 98, 100);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithExactlyMaxCosignatures) {
		AssertMaxCosignaturesValidationResult(ValidationResult::Success, 99, 100);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithGreaterThanMaxCosignatures) {
		AssertMaxCosignaturesValidationResult(Failure_Aggregate_Too_Many_Cosignatures, 100, 100);
		AssertMaxCosignaturesValidationResult(Failure_Aggregate_Too_Many_Cosignatures, 222, 100);
	}

	// endregion

	// region cosignatory uniqueness

	namespace {
		void AssertCosignatoryUniquenessValidationResult(
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

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithAllCosignatoriesBeingUnique) {
		// Arrange: no conflicts
		auto signer = test::GenerateRandomByteArray<Key>();
		auto cosignatures = GenerateRandomCosignatures(5);

		// Assert:
		AssertCosignatoryUniquenessValidationResult(ValidationResult::Success, signer, cosignatures);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithRedundantExplicitAndImplicitCosignatory) {
		// Arrange:
		auto signer = test::GenerateRandomByteArray<Key>();
		auto cosignatures = GenerateRandomCosignatures(5);
		cosignatures[2].SignerPublicKey = signer;

		// Assert:
		AssertCosignatoryUniquenessValidationResult(Failure_Aggregate_Redundant_Cosignatures, signer, cosignatures);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithRedundantImplicitCosignatories) {
		// Arrange:
		auto signer = test::GenerateRandomByteArray<Key>();
		auto cosignatures = GenerateRandomCosignatures(5);
		cosignatures[0].SignerPublicKey = cosignatures[4].SignerPublicKey;

		// Assert:
		AssertCosignatoryUniquenessValidationResult(Failure_Aggregate_Redundant_Cosignatures, signer, cosignatures);
	}

	// endregion
}}
