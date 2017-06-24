#include "src/model/MosaicLevyChangeTransaction.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS MosaicLevyChangeTransactionTests

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(Transaction) // base
				+ sizeof(MosaicId) // mosaic id
				+ sizeof(MosaicId) // mosaic levy id
				+ sizeof(Address) // recipient
				+ sizeof(uint8_t) * Num_Mosaic_Levy_Rule_Ids // rule ids
				+ sizeof(uint8_t); // number of rule definitions

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(MosaicLevyChangeTransaction));
		EXPECT_EQ(120u + 47u, sizeof(MosaicLevyChangeTransaction));
	}

	TEST(TEST_CLASS, TransactionHasExpectedProperties) {
		// Assert:
		EXPECT_EQ(EntityType::Mosaic_Levy_Change, static_cast<EntityType>(MosaicLevyChangeTransaction::Entity_Type));
		EXPECT_EQ(1u, static_cast<uint8_t>(MosaicLevyChangeTransaction::Current_Version));
	}

	// region data pointers

	namespace {
		struct MosaicLevyChangeTransactionTraits {
			static auto GenerateEntityWithAttachments(uint16_t count) {
				uint32_t entitySize = sizeof(MosaicLevyChangeTransaction) + count * sizeof(RuleDefinition);
				std::unique_ptr<MosaicLevyChangeTransaction> pTransaction(
						reinterpret_cast<MosaicLevyChangeTransaction*>(::operator new(entitySize)));
				pTransaction->Size = entitySize;
				pTransaction->RuleDefinitionCount = static_cast<uint8_t>(count);
				return pTransaction;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.RuleDefinitionPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, MosaicLevyChangeTransactionTraits) // RuleDefinitionPtr

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		MosaicLevyChangeTransaction transaction;
		transaction.Size = 0;
		transaction.RuleDefinitionCount = 100;

		// Act:
		auto realSize = MosaicLevyChangeTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(24u, sizeof(RuleDefinition));
		EXPECT_EQ(sizeof(MosaicLevyChangeTransaction) + 100 * sizeof(RuleDefinition), realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		MosaicLevyChangeTransaction transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.RuleDefinitionCount);

		// Act:
		auto realSize = MosaicLevyChangeTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(MosaicLevyChangeTransaction) + transaction.RuleDefinitionCount * sizeof(RuleDefinition), realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
