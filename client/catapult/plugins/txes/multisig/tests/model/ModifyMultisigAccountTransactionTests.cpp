#include "src/model/ModifyMultisigAccountTransaction.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	using TransactionType = ModifyMultisigAccountTransaction;

#define TEST_CLASS ModifyMultisigAccountTransactionTests

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto expectedSize = baseSize // base
				+ sizeof(int8_t) // min cosignatories removal delta
				+ sizeof(int8_t) // min cosignatories delta
				+ sizeof(uint8_t); // modifications count

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 3u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(EntityType::Modify_Multisig_Account, static_cast<EntityType>(T::Entity_Type));
			EXPECT_EQ(3u, static_cast<uint8_t>(T::Current_Version));
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(ModifyMultisigAccount);

	// endregion

	// region data pointers

	namespace {
		struct ModifyMultisigAccountTransactionTraits {
			static auto GenerateEntityWithAttachments(uint8_t numModifications) {
				uint32_t entitySize = sizeof(TransactionType) + numModifications * sizeof(CosignatoryModification);
				std::unique_ptr<TransactionType> pTransaction(reinterpret_cast<TransactionType*>(::operator new(entitySize)));
				pTransaction->Size = entitySize;
				pTransaction->ModificationsCount = numModifications;
				return pTransaction;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.ModificationsPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, ModifyMultisigAccountTransactionTraits);

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		TransactionType transaction;
		transaction.Size = 0;
		transaction.ModificationsCount = 11;

		// Act:
		auto realSize = TransactionType::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(33u, sizeof(CosignatoryModification));
		EXPECT_EQ(sizeof(TransactionType) + 11 * sizeof(CosignatoryModification), realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		TransactionType transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.ModificationsCount);

		// Act:
		auto realSize = TransactionType::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(TransactionType) + 0xFF * sizeof(CosignatoryModification), realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
