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
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS VerifiableEntityTests

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(uint32_t) // size
				+ sizeof(Signature) // signature
				+ sizeof(uint16_t) // version
				+ sizeof(uint16_t) // entity type
				+ sizeof(Key); // signer

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(VerifiableEntity));
		EXPECT_EQ(104u, sizeof(VerifiableEntity));
	}

	// region insertion operator

	TEST(TEST_CLASS, CanOutputEntity) {
		// Arrange:
		VerifiableEntity entity;
		entity.Size = 121;
		entity.Type = Entity_Type_Nemesis_Block;
		entity.Version = MakeVersion(NetworkIdentifier::Zero, 2);

		// Act:
		auto str = test::ToString(entity);

		// Assert:
		EXPECT_EQ("Nemesis_Block (v2) with size 121", str);
	}

	// endregion

	namespace {
		bool IsSizeValid(const VerifiableEntity& entity) {
			auto registry = mocks::CreateDefaultTransactionRegistry();
			return IsSizeValid(entity, registry);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Transaction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::TransactionPolicy>(); } \
	TEST(TEST_CLASS, TEST_NAME##_EmptyBlock) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::EmptyBlockPolicy>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonEmptyBlock) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::NonEmptyBlockPolicy>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

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
		// Assert:
		AssertFailureForBlockWithEntityType(static_cast<EntityType>(1234));
	}

	TEST(TEST_CLASS, SizeIsInvalidWhenValidatingBlockContainingIncompatibleEntityType) {
		// Assert:
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
