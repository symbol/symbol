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

#include "src/extensions/TransactionExtensions.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS TransactionExtensionsTests

	// region traits

	namespace {
		struct NormalTraits {
			static constexpr size_t Entity_Size = sizeof(model::Transaction);
			static constexpr auto Entity_Type = mocks::MockTransaction::Entity_Type;
			static constexpr auto Is_Aggregate = false;
		};
		struct LargeTraits {
			static constexpr size_t Entity_Size = sizeof(model::Transaction) + 123;
			static constexpr auto Entity_Type = mocks::MockTransaction::Entity_Type;
			static constexpr auto Is_Aggregate = false;
		};
		struct AggregateBondedTraits {
			static constexpr size_t Entity_Size = sizeof(model::AggregateTransaction);
			static constexpr auto Entity_Type = model::Entity_Type_Aggregate_Bonded;
			static constexpr auto Is_Aggregate = true;
		};
		struct AggregateCompleteTraits {
			static constexpr size_t Entity_Size = sizeof(model::AggregateTransaction) + 3 * sizeof(model::Cosignature);
			static constexpr auto Entity_Type = model::Entity_Type_Aggregate_Complete;
			static constexpr auto Is_Aggregate = true;
		};
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Normal) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NormalTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Large) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<LargeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_AggregateBonded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AggregateBondedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_AggregateComplete) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AggregateCompleteTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region hash tests

	TRAITS_BASED_TEST(TransactionHashIsDependentOnGenerationHash) {
		// Arrange:
		TransactionExtensions extensions1(test::GenerateRandomByteArray<GenerationHashSeed>());
		TransactionExtensions extensions2(test::GenerateRandomByteArray<GenerationHashSeed>());
		auto pEntity = test::GenerateRandomTransactionWithSize(TTraits::Entity_Size);
		pEntity->Type = TTraits::Entity_Type;

		// Act:
		auto hash1 = extensions1.hash(*pEntity);
		auto hash2 = extensions2.hash(*pEntity);

		// Assert:
		EXPECT_NE(hash1, hash2);
	}

	TRAITS_BASED_TEST(TransactionHashIsDependentOnTransactionData) {
		// Arrange:
		TransactionExtensions extensions(test::GenerateRandomByteArray<GenerationHashSeed>());
		auto pEntity1 = test::GenerateRandomTransactionWithSize(TTraits::Entity_Size);
		pEntity1->Type = TTraits::Entity_Type;

		auto pEntity2 = test::GenerateRandomTransactionWithSize(TTraits::Entity_Size);
		pEntity2->Type = TTraits::Entity_Type;

		// Act:
		auto hash1 = extensions.hash(*pEntity1);
		auto hash2 = extensions.hash(*pEntity2);

		// Assert:
		EXPECT_NE(hash1, hash2);
	}

	TRAITS_BASED_TEST(TransactionHashIsCalculatedFromCorrectData) {
		// Arrange:
		TransactionExtensions extensions(test::GenerateRandomByteArray<GenerationHashSeed>());
		auto pEntity1 = test::GenerateRandomTransactionWithSize(TTraits::Entity_Size);
		pEntity1->Type = TTraits::Entity_Type;

		// - copy the entity and change the terminal byte
		auto pEntity2 = test::CopyEntity(*pEntity1);
		*(reinterpret_cast<uint8_t*>(pEntity2.get()) + TTraits::Entity_Size - 1) ^= 0xFF;

		// Act:
		auto hash1 = extensions.hash(*pEntity1);
		auto hash2 = extensions.hash(*pEntity2);

		// Assert: the terminal byte shouldn't influence an aggregate hash but should influence other hashes
		if (TTraits::Is_Aggregate)
			EXPECT_EQ(hash1, hash2);
		else
			EXPECT_NE(hash1, hash2);
	}

	// endregion

	// region sign / verify tests

	namespace {
		template<typename TTraits>
		bool RunSignVerifyAction(const consumer<model::Transaction&>& modifier) {
			// Arrange: create a signed transaction
			auto signer = test::GenerateKeyPair();

			TransactionExtensions extensions(test::GenerateRandomByteArray<GenerationHashSeed>());
			auto pEntity = test::GenerateRandomTransactionWithSize(TTraits::Entity_Size);
			pEntity->Type = TTraits::Entity_Type;
			pEntity->SignerPublicKey = signer.publicKey();

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
		auto signer = test::GenerateKeyPair();

		TransactionExtensions extensions1(test::GenerateRandomByteArray<GenerationHashSeed>());
		TransactionExtensions extensions2(test::GenerateRandomByteArray<GenerationHashSeed>());
		auto pEntity = test::GenerateRandomTransactionWithSize(TTraits::Entity_Size);
		pEntity->Type = TTraits::Entity_Type;
		pEntity->SignerPublicKey = signer.publicKey();

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
		auto generationHashSeed = utils::ParseByteArray<GenerationHashSeed>(test::Deterministic_Network_Generation_Hash_Seed_String);
		auto pTransaction = test::GenerateDeterministicTransaction();

		// Act:
		auto isVerified = TransactionExtensions(generationHashSeed).verify(*pTransaction);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	// endregion
}}
