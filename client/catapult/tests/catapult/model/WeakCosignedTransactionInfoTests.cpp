#include "catapult/model/WeakCosignedTransactionInfo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS WeakCosignedTransactionInfoTests

	TEST(TEST_CLASS, CanCreateUnsetWeakCosignedTransactionInfo) {
		// Act:
		WeakCosignedTransactionInfo transactionInfo;

		// Assert:
		EXPECT_FALSE(!!transactionInfo);
	}

	TEST(TEST_CLASS, CanCreateWeakCosignedTransactionInfo) {
		// Act:
		Transaction transaction;
		auto cosignatures = test::GenerateRandomDataVector<Cosignature>(3);
		WeakCosignedTransactionInfo transactionInfo(&transaction, &cosignatures);

		// Assert:
		EXPECT_TRUE(!!transactionInfo);
		EXPECT_EQ(&transaction, &transactionInfo.transaction());
		EXPECT_EQ(&cosignatures, &transactionInfo.cosignatures());
	}

	TEST(TEST_CLASS, HasCosignerReturnsTrueWhenSignerIsCosigner) {
		// Arrange:
		Transaction transaction;
		auto cosignatures = test::GenerateRandomDataVector<Cosignature>(3);
		WeakCosignedTransactionInfo transactionInfo(&transaction, &cosignatures);

		// Act + Assert:
		for (const auto& cosignature : cosignatures)
			EXPECT_TRUE(transactionInfo.hasCosigner(cosignature.Signer));
	}

	TEST(TEST_CLASS, HasCosignerReturnsFalseWhenSignerIsNotCosigner) {
		// Arrange:
		Transaction transaction;
		transaction.Signer = test::GenerateRandomData<Key_Size>();
		auto cosignatures = test::GenerateRandomDataVector<Cosignature>(3);
		WeakCosignedTransactionInfo transactionInfo(&transaction, &cosignatures);

		// Act + Assert:
		EXPECT_FALSE(transactionInfo.hasCosigner(transaction.Signer));
		EXPECT_FALSE(transactionInfo.hasCosigner(test::GenerateRandomData<Key_Size>()));
	}
}}
