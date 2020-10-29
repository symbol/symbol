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

#include "catapult/model/VerifiableEntity.h"
#include "catapult/model/Address.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS VerifiableEntityTests

	// region size + alignment

#define VERIFIABLE_ENTITY_FIELDS FIELD(Signature) FIELD(SignerPublicKey) FIELD(Version) FIELD(Network) FIELD(Type)

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(SizePrefixedEntity) + 2 * sizeof(uint32_t);

#define FIELD(X) expectedSize += SizeOf32<decltype(VerifiableEntity::X)>();
		VERIFIABLE_ENTITY_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(VerifiableEntity));
		EXPECT_EQ(4u + 8 + 100, sizeof(VerifiableEntity));
	}

	TEST(TEST_CLASS, EntityHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(VerifiableEntity, X);
		VERIFIABLE_ENTITY_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(VerifiableEntity) % 8);
	}

#undef VERIFIABLE_ENTITY_FIELDS

	// endregion

	// region insertion operator

	TEST(TEST_CLASS, CanOutputEntity) {
		// Arrange:
		VerifiableEntity entity;
		entity.Size = 121;
		entity.Version = 2;
		entity.Type = Entity_Type_Nemesis_Block;

		// Act:
		auto str = test::ToString(entity);

		// Assert:
		EXPECT_EQ("Nemesis_Block (v2) with size 121", str);
	}

	// endregion

	// region GetSignerAddress

	TEST(TEST_CLASS, GetSignerAddressCalculatesCorrectSignerAddress) {
		// Arrange:
		VerifiableEntity entity;
		test::FillWithRandomData(entity.SignerPublicKey);
		entity.Network = static_cast<NetworkIdentifier>(test::RandomByte());

		// Act:
		auto signerAddress = GetSignerAddress(entity);

		// Assert:
		auto expectedSignerAddress = PublicKeyToAddress(entity.SignerPublicKey, entity.Network);
		EXPECT_EQ(expectedSignerAddress, signerAddress);
	}

	// endregion

	// region IsSizeValid - utils

	namespace {
		bool IsSizeValid(const VerifiableEntity& entity) {
			auto registry = mocks::CreateDefaultTransactionRegistry();
			return IsSizeValid(entity, registry);
		}

		struct TransactionTraits {
			static auto Create() {
				return test::GenerateRandomTransaction();
			}
		};

		struct EmptyBlockTraits {
			static auto Create() {
				return test::GenerateEmptyRandomBlock();
			}
		};

		struct BlockWithTransactionsTraits {
			static auto Create() {
				auto transactions = test::GenerateRandomTransactions(3);
				return test::GenerateBlockWithTransactions(transactions);
			}
		};
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Transaction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_EmptyBlock) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmptyBlockTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_BlockWithTransactions) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockWithTransactionsTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region IsSizeValid - basic tests

	TRAITS_BASED_TEST(SizeIsValidWhenEntitySizeIsCorrect) {
		// Arrange:
		auto pEntity = TTraits::Create();

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pEntity));
	}

	TRAITS_BASED_TEST(SizeIsInvalidWhenEntitySizeIsTooSmall) {
		// Arrange:
		auto pEntity = TTraits::Create();
		--pEntity->Size;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pEntity));
	}

	TRAITS_BASED_TEST(SizeIsInvalidWhenEntitySizeIsTooLarge) {
		// Arrange:
		auto pEntity = TTraits::Create();
		++pEntity->Size;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pEntity));
	}

	TEST(TEST_CLASS, SizeIsInvalidForEntityWithReportedSizeLessThanHeaderSize) {
		// Arrange:
		std::vector<uint8_t> buffer(sizeof(SizePrefixedEntity));
		auto* pEntity = reinterpret_cast<VerifiableEntity*>(&buffer[0]);
		pEntity->Size = sizeof(SizePrefixedEntity);

		// Act:
		EXPECT_FALSE(IsSizeValid(*pEntity));
	}

	// endregion

	// region IsSizeValid - block

	namespace {
		void AssertFailureForBlockWithEntityType(EntityType type) {
			// Act:
			auto pBlock = test::GenerateBlockWithTransactions(test::GenerateRandomTransactions(1));
			auto transactions = pBlock->Transactions();
			transactions.begin()->Type = type;

			// Act + Assert:
			EXPECT_FALSE(IsSizeValid(*pBlock));
		}
	}

	TEST(TEST_CLASS, SizeIsInvalidWhenValidatingBlockContainingUnknownEntityType) {
		AssertFailureForBlockWithEntityType(static_cast<EntityType>(1234));
	}

	TEST(TEST_CLASS, SizeIsInvalidWhenValidatingBlockContainingIncompatibleEntityType) {
		AssertFailureForBlockWithEntityType(Entity_Type_Nemesis_Block);
	}

	TEST(TEST_CLASS, SizeIsInvalidWhenValidatingBlockContainingTransactionWithWrongSize) {
		// Act:
		auto pBlock = test::GenerateBlockWithTransactions(test::GenerateRandomTransactions(1));
		auto transactions = pBlock->Transactions();
		transactions.begin()->Size += 123;

		// Assert:// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	// endregion
}}
