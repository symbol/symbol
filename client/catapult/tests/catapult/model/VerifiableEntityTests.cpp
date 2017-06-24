#include "catapult/model/VerifiableEntity.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS VerifiableEntityTests

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(uint32_t) // size
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
		entity.Type = EntityType::Transfer;
		entity.Version = MakeVersion(NetworkIdentifier::Zero, 2);

		// Act:
		auto str = test::ToString(entity);

		// Assert:
		EXPECT_EQ("Transfer (v2) with size 121", str);
	}

	// endregion

	// region IsSizeValid

	namespace {
		bool IsSizeValid(const VerifiableEntity& entity) {
			auto pRegistry = mocks::CreateDefaultTransactionRegistry();
			return IsSizeValid(entity, *pRegistry);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Transaction) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::TransactionPolicy>(); } \
	TEST(TEST_CLASS, TEST_NAME##_EmptyBlock) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::EmptyBlockPolicy>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonEmptyBlock) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::NonEmptyBlockPolicy>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region basic tests

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

	// region block

	namespace {
		void AssertFailureForBlockWithEntityType(EntityType type) {
			// Act:
			auto pBlock = test::GenerateRandomBlockWithTransactions(test::GenerateRandomTransactions(1));
			auto transactions = pBlock->Transactions();
			transactions.begin()->Type = type;

			// Act + Assert:
			EXPECT_FALSE(IsSizeValid(*pBlock));
		}
	}

	TEST(TEST_CLASS, SizeIsInvalidWhenValidatingBlockContainingUnknownEntityType) {
		// Assert:
		AssertFailureForBlockWithEntityType(model::EntityType(1234));
	}

	TEST(TEST_CLASS, SizeIsInvalidWhenValidatingBlockContainingIncompatibleEntityType) {
		// Assert:
		AssertFailureForBlockWithEntityType(model::EntityType::Nemesis_Block);
	}

	TEST(TEST_CLASS, SizeIsInvalidWhenValidatingBlockContainingTransactionWithWrongSize) {
		// Act:
		auto pBlock = test::GenerateRandomBlockWithTransactions(test::GenerateRandomTransactions(1));
		auto transactions = pBlock->Transactions();
		transactions.begin()->Size += 123;

		// Assert:// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	// endregion

	// endregion
}}
