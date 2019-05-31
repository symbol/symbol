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
#include "catapult/utils/HexParser.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS TransactionExtensionsTests

	// region traits

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

	// endregion

	// region basic sign / verify tests

	namespace {
		template<typename TTraits>
		bool RunSignVerifyAction(const consumer<model::Transaction&>& modifier) {
			// Arrange: create a signed transaction
			TransactionExtensions extensions(test::GenerateRandomByteArray<GenerationHash>());
			auto pEntity = test::GenerateRandomTransactionWithSize(TTraits::Entity_Size);
			auto signer = test::GenerateKeyPair();
			pEntity->Signer = signer.publicKey();
			extensions.sign(signer, *pEntity);

			// - modify the transaction
			modifier(*pEntity);

			// Act:
			return extensions.verify(*pEntity);
		}
	}

	TRAITS_BASED_TEST(CannotValidateUnsignedTransaction) {
		// Act:
		auto result = RunSignVerifyAction<TTraits>([](auto& transaction) {
			transaction.Signature = {};
		});

		// Assert:
		EXPECT_FALSE(result);
	}

	TRAITS_BASED_TEST(SignedTransactionValidates) {
		// Act:
		auto result = RunSignVerifyAction<TTraits>([](const auto&) {});

		// Assert:
		EXPECT_TRUE(result);
	}

	TRAITS_BASED_TEST(CannotValidateAlteredSignedTransaction) {
		// Act:
		auto result = RunSignVerifyAction<TTraits>([](auto& transaction) {
			transaction.Deadline = Timestamp(transaction.Deadline.unwrap() ^ 0xFFFF'FFFF'FFFF'FFFFull);
		});

		// Assert:
		EXPECT_FALSE(result);
	}

	TRAITS_BASED_TEST(CannotValidateSignedTransactionWithAlteredSignature) {
		// Act:
		auto result = RunSignVerifyAction<TTraits>([](auto& transaction) {
			transaction.Signature[0] ^= 0xFFu;
		});

		// Assert:
		EXPECT_FALSE(result);
	}

	TRAITS_BASED_TEST(CannotValidateSignedTransactionWithAlteredGenerationHash) {
		// Arrange:
		TransactionExtensions extensions1(test::GenerateRandomByteArray<GenerationHash>());
		TransactionExtensions extensions2(test::GenerateRandomByteArray<GenerationHash>());
		auto pEntity = test::GenerateRandomTransactionWithSize(TTraits::Entity_Size);
		auto signer = test::GenerateKeyPair();
		pEntity->Signer = signer.publicKey();
		extensions1.sign(signer, *pEntity);

		// Sanity:
		EXPECT_TRUE(extensions1.verify(*pEntity));

		// Act + Assert:
		EXPECT_FALSE(extensions2.verify(*pEntity));
	}

	// endregion

	// region Deterministic Entity Sanity

	TEST(TEST_CLASS, DeterministicTransactionIsFullyVerifiable) {
		// Arrange:
		auto generationHash = utils::ParseByteArray<GenerationHash>(test::Deterministic_Network_Generation_Hash_String);
		auto pTransaction = test::GenerateDeterministicTransaction();

		// Act:
		auto isVerified = TransactionExtensions(generationHash).verify(*pTransaction);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	// endregion
}}
