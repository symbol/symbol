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

#include "src/model/ModifyMultisigAccountTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/MultisigTestUtils.h"
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
			auto expectedSize =
					baseSize // base
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
			EXPECT_EQ(Entity_Type_Modify_Multisig_Account, T::Entity_Type);
			EXPECT_EQ(1u, T::Current_Version);
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(ModifyMultisigAccount)

	// endregion

	// region data pointers

	namespace {
		struct ModifyMultisigAccountTransactionTraits {
			static auto GenerateEntityWithAttachments(uint8_t numModifications) {
				uint32_t entitySize = sizeof(TransactionType) + numModifications * sizeof(CosignatoryModification);
				auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
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

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, ModifyMultisigAccountTransactionTraits)

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
		ASSERT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(TransactionType) + 0xFF * sizeof(CosignatoryModification), realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion

	// region ExtractAdditionalRequiredCosigners

	namespace {
		auto CreateEmbeddedTransactionWithModifications(const std::vector<CosignatoryModificationType>& modificationTypes) {
			return test::CreateModifyMultisigAccountTransaction(test::GenerateRandomByteArray<Key>(), modificationTypes);
		}
	}

	TEST(TEST_CLASS, ExtractAdditionalRequiredCosigners_NoModifications) {
		// Arrange:
		auto pTransaction = CreateEmbeddedTransactionWithModifications({});

		// Act:
		auto additionalCosigners = ExtractAdditionalRequiredCosigners(*pTransaction);

		// Assert:
		EXPECT_EQ(utils::KeySet(), additionalCosigners);
	}

	TEST(TEST_CLASS, ExtractAdditionalRequiredCosigners_AddModifications) {
		// Arrange:
		auto pTransaction = CreateEmbeddedTransactionWithModifications({
			CosignatoryModificationType::Add,
			CosignatoryModificationType::Add
		});

		// Act:
		auto additionalCosigners = ExtractAdditionalRequiredCosigners(*pTransaction);

		// Assert:
		const auto* pModifications = pTransaction->ModificationsPtr();
		EXPECT_EQ(utils::KeySet({ pModifications[0].CosignatoryPublicKey, pModifications[1].CosignatoryPublicKey }), additionalCosigners);
	}

	TEST(TEST_CLASS, ExtractAdditionalRequiredCosigners_DelModifications) {
		// Arrange:
		auto pTransaction = CreateEmbeddedTransactionWithModifications({
			CosignatoryModificationType::Del,
			CosignatoryModificationType::Del
		});

		// Act:
		auto additionalCosigners = ExtractAdditionalRequiredCosigners(*pTransaction);

		// Assert:
		EXPECT_EQ(utils::KeySet(), additionalCosigners);
	}

	TEST(TEST_CLASS, ExtractAdditionalRequiredCosigners_AddAndDelModifications) {
		// Arrange:
		auto pTransaction = CreateEmbeddedTransactionWithModifications({
			CosignatoryModificationType::Add,
			CosignatoryModificationType::Del,
			CosignatoryModificationType::Add,
			CosignatoryModificationType::Del
		});

		// Act:
		auto additionalCosigners = ExtractAdditionalRequiredCosigners(*pTransaction);

		// Assert:
		const auto* pModifications = pTransaction->ModificationsPtr();
		EXPECT_EQ(utils::KeySet({ pModifications[0].CosignatoryPublicKey, pModifications[2].CosignatoryPublicKey }), additionalCosigners);
	}

	// endregion
}}
