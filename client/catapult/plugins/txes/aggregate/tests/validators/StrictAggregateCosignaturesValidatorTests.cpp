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

#define TEST_CLASS StrictAggregateCosignaturesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(StrictAggregateCosignatures,)

	namespace {
		using Keys = std::vector<Key>;

		void AssertValidationResult(ValidationResult expectedResult, const Key& signer, const Keys& cosigners, const Keys& txSigners) {
			// Arrange:
			// - setup transactions
			std::vector<uint8_t> txBuffer(sizeof(model::EmbeddedTransaction) * txSigners.size());
			auto* pTransactions = reinterpret_cast<model::EmbeddedTransaction*>(txBuffer.data());
			for (auto i = 0u; i < txSigners.size(); ++i) {
				pTransactions[i].Signer = txSigners[i];
				pTransactions[i].Size = sizeof(model::EmbeddedTransaction);
			}

			// - setup cosignatures
			auto cosignatures = test::GenerateRandomDataVector<model::Cosignature>(cosigners.size());
			for (auto i = 0u; i < cosigners.size(); ++i)
				cosignatures[i].Signer = cosigners[i];

			using Notification = model::AggregateCosignaturesNotification;
			Notification notification(signer, txSigners.size(), pTransactions, cosigners.size(), cosignatures.data());
			auto pValidator = CreateStrictAggregateCosignaturesValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// region success

	TEST(TEST_CLASS, SuccessWhenTransactionSignersExactlyMatchCosigners) {
		// Arrange:
		auto signer = test::GenerateRandomByteArray<Key>();
		auto cosigners = test::GenerateRandomDataVector<Key>(3);
		auto txSigners = Keys{ signer, cosigners[0], cosigners[1], cosigners[2] };

		// Assert:
		AssertValidationResult(ValidationResult::Success, signer, cosigners, txSigners);
	}

	TEST(TEST_CLASS, SuccessWhenTransactionSignersExactlyMatchCosignersOutOfOrder) {
		// Arrange:
		auto signer = test::GenerateRandomByteArray<Key>();
		auto cosigners = test::GenerateRandomDataVector<Key>(3);
		auto txSigners = Keys{ cosigners[2], cosigners[0], signer, cosigners[1] };

		// Assert:
		AssertValidationResult(ValidationResult::Success, signer, cosigners, txSigners);
	}

	TEST(TEST_CLASS, SuccessWhenAllTransactionsHaveSameSignerAsAggregate) {
		// Arrange:
		auto signer = test::GenerateRandomByteArray<Key>();
		auto txSigners = Keys{ signer, signer, signer };

		// Assert:
		AssertValidationResult(ValidationResult::Success, signer, {}, txSigners);
	}

	// endregion

	// region failure

	TEST(TEST_CLASS, FailureWhenTransactionSignerIsNotMachedByCosigner) {
		// Arrange: there is an extra tx signer with no match
		auto signer = test::GenerateRandomByteArray<Key>();
		auto cosigners = test::GenerateRandomDataVector<Key>(3);
		auto txSigners = Keys{ signer, cosigners[0], cosigners[1], cosigners[2], test::GenerateRandomByteArray<Key>() };

		// Assert:
		AssertValidationResult(Failure_Aggregate_Missing_Cosigners, signer, cosigners, txSigners);
	}

	TEST(TEST_CLASS, FailureWhenCosignerIsNotTransactionSigner) {
		// Arrange: there is a cosigner that doesn't match any tx
		auto signer = test::GenerateRandomByteArray<Key>();
		auto cosigners = test::GenerateRandomDataVector<Key>(3);
		auto txSigners = Keys{ signer, cosigners[0], cosigners[2] };

		// Assert:
		AssertValidationResult(Failure_Aggregate_Ineligible_Cosigners, signer, cosigners, txSigners);
	}

	TEST(TEST_CLASS, FailureIneligibleDominatesFailureMissing) {
		// Arrange: there is an extra tx signer with no match and there is a cosigner that doesn't match any tx
		auto signer = test::GenerateRandomByteArray<Key>();
		auto cosigners = test::GenerateRandomDataVector<Key>(3);
		auto txSigners = Keys{ signer, cosigners[0], cosigners[2], test::GenerateRandomByteArray<Key>() };

		// Assert:
		AssertValidationResult(Failure_Aggregate_Ineligible_Cosigners, signer, cosigners, txSigners);
	}

	// endregion
}}
