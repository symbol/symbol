#include "src/validators/Validators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS StrictAggregateCosignaturesValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(StrictAggregateCosignatures,)

	namespace {
		void AssertValidationResult(
				ValidationResult expectedResult,
				const Key& signer,
				const std::vector<Key>& cosigners,
				const std::vector<Key>& txSigners) {
			// Arrange:
			// - setup transactions
			std::vector<uint8_t> txBuffer(sizeof(model::EmbeddedEntity) * txSigners.size());
			auto* pTransactions = reinterpret_cast<model::EmbeddedEntity*>(txBuffer.data());
			for (auto i = 0u; i < txSigners.size(); ++i) {
				pTransactions[i].Signer = txSigners[i];
				pTransactions[i].Size = sizeof(model::EmbeddedEntity);
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
		auto signer = test::GenerateRandomData<Key_Size>();
		auto cosigners = test::GenerateRandomDataVector<Key>(3);
		auto txSigners = std::vector<Key>{ signer, cosigners[0], cosigners[1], cosigners[2] };

		// Assert:
		AssertValidationResult(ValidationResult::Success, signer, cosigners, txSigners);
	}

	TEST(TEST_CLASS, SuccessWhenTransactionSignersExactlyMatchCosignersOutOfOrder) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto cosigners = test::GenerateRandomDataVector<Key>(3);
		auto txSigners = std::vector<Key>{ cosigners[2], cosigners[0], signer, cosigners[1] };

		// Assert:
		AssertValidationResult(ValidationResult::Success, signer, cosigners, txSigners);
	}

	TEST(TEST_CLASS, SuccessWhenAllTransactionsHaveSameSignerAsAggregate) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto txSigners = std::vector<Key>{ signer, signer, signer };

		// Assert:
		AssertValidationResult(ValidationResult::Success, signer, {}, txSigners);
	}

	// endregion

	// region failure

	TEST(TEST_CLASS, FailureWhenTransactionSignerIsNotMachedByCosigner) {
		// Arrange: there is an extra tx signer with no match
		auto signer = test::GenerateRandomData<Key_Size>();
		auto cosigners = test::GenerateRandomDataVector<Key>(3);
		auto txSigners = std::vector<Key>{ signer, cosigners[0], cosigners[1], cosigners[2], test::GenerateRandomData<Key_Size>() };

		// Assert:
		AssertValidationResult(Failure_Aggregate_Missing_Cosigners, signer, cosigners, txSigners);
	}

	TEST(TEST_CLASS, FailureWhenCosignerIsNotTransactionSigner) {
		// Arrange: there is a cosigner that doesn't match any tx
		auto signer = test::GenerateRandomData<Key_Size>();
		auto cosigners = test::GenerateRandomDataVector<Key>(3);
		auto txSigners = std::vector<Key>{ signer, cosigners[0], cosigners[2] };

		// Assert:
		AssertValidationResult(Failure_Aggregate_Ineligible_Cosigners, signer, cosigners, txSigners);
	}

	// endregion
}}
