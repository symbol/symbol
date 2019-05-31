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
		transaction.Signer = test::GenerateRandomByteArray<Key>();
		auto cosignatures = test::GenerateRandomDataVector<Cosignature>(3);
		WeakCosignedTransactionInfo transactionInfo(&transaction, &cosignatures);

		// Act + Assert:
		EXPECT_FALSE(transactionInfo.hasCosigner(transaction.Signer));
		EXPECT_FALSE(transactionInfo.hasCosigner(test::GenerateRandomByteArray<Key>()));
	}
}}
