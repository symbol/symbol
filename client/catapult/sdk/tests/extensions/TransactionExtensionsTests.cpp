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

#include "src/extensions/TransactionExtensions.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS TransactionExtensionsTests

	namespace {
		struct NormalTraits {
			static constexpr size_t Entity_Size = sizeof(model::Transaction);
		};
		struct LargeTraits {
			static constexpr size_t Entity_Size = sizeof(model::Transaction) + 123;
		};
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Normal) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NormalTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Large) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LargeTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(CannotValidateUnsignedTransaction) {
		// Arrange:
		auto pEntity = test::GenerateRandomTransaction(TTraits::Entity_Size);

		// Act + Assert:
		EXPECT_FALSE(VerifyTransactionSignature(*pEntity));
	}

	TRAITS_BASED_TEST(SignedTransactionValidates) {
		// Arrange:
		auto pEntity = test::GenerateRandomTransaction(TTraits::Entity_Size);
		auto signer = test::GenerateKeyPair();
		(*pEntity).Signer = signer.publicKey();
		SignTransaction(signer, *pEntity);

		// Act + Assert:
		EXPECT_TRUE(VerifyTransactionSignature(*pEntity));
	}

	TRAITS_BASED_TEST(CannotValidateAlteredSignedTransaction) {
		// Arrange:
		auto pEntity = test::GenerateRandomTransaction(TTraits::Entity_Size);
		auto signer = test::GenerateKeyPair();
		(*pEntity).Signer = signer.publicKey();
		SignTransaction(signer, *pEntity);

		(*pEntity).Deadline = Timestamp(pEntity->Deadline.unwrap() ^ 0xFFFF'FFFF'FFFF'FFFFull);

		// Act + Assert:
		EXPECT_FALSE(VerifyTransactionSignature(*pEntity));
	}

	TRAITS_BASED_TEST(CannotValidateSignedTransactionWithAlteredSignature) {
		// Arrange:
		auto pEntity = test::GenerateRandomTransaction(TTraits::Entity_Size);
		auto signer = test::GenerateKeyPair();
		(*pEntity).Signer = signer.publicKey();
		SignTransaction(signer, *pEntity);

		(*pEntity).Signature[0] ^= 0xFFu;

		// Act + Assert:
		EXPECT_FALSE(VerifyTransactionSignature(*pEntity));
	}

	// region Deterministic Entity Sanity

	TEST(TEST_CLASS, DeterministicTransactionIsFullyVerifiable) {
		// Arrange:
		auto pTransaction = test::GenerateDeterministicTransaction();

		// Act:
		auto isVerified = VerifyTransactionSignature(*pTransaction);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	// endregion
}}
