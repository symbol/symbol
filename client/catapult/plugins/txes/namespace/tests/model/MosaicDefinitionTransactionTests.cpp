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

#include "src/model/MosaicDefinitionTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS MosaicDefinitionTransactionTests

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto propertiesHeaderSize = sizeof(uint8_t) // count
					+ sizeof(uint8_t) // flags
					+ sizeof(uint8_t); // divisibility
			auto expectedSize = baseSize // base
					+ sizeof(NamespaceId) // parent namespace id
					+ sizeof(MosaicId) // id
					+ sizeof(uint8_t) // mosaic name size
					+ propertiesHeaderSize;

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 20u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Mosaic_Definition, static_cast<EntityType>(T::Entity_Type));
			EXPECT_EQ(2u, static_cast<uint8_t>(T::Current_Version));
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(MosaicDefinition)

	// endregion

	// region data pointers

	namespace {
		struct MosaicDefinitionTransactionTraits {
			static auto GenerateEntityWithAttachments(uint8_t nameSize, uint8_t propertiesCount) {
				uint32_t entitySize = sizeof(MosaicDefinitionTransaction)
						+ propertiesCount * sizeof(model::MosaicProperty)
						+ nameSize;
				auto pTransaction = utils::MakeUniqueWithSize<MosaicDefinitionTransaction>(entitySize);
				pTransaction->Size = entitySize;
				pTransaction->MosaicNameSize = nameSize;
				pTransaction->PropertiesHeader.Count = propertiesCount;
				return pTransaction;
			}

			static constexpr size_t GetAttachment1Size(uint16_t nameSize) {
				return nameSize;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer1(TEntity& entity) {
				return entity.NamePtr();
			}

			template<typename TEntity>
			static auto GetAttachmentPointer2(TEntity& entity) {
				return entity.PropertiesPtr();
			}
		};
	}

	DEFINE_DUAL_ATTACHMENT_POINTER_TESTS(TEST_CLASS, MosaicDefinitionTransactionTraits)

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		MosaicDefinitionTransaction transaction;
		transaction.Size = 0;
		transaction.PropertiesHeader.Count = 33;
		transaction.MosaicNameSize = 44;

		// Act:
		auto realSize = MosaicDefinitionTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(9u, sizeof(model::MosaicProperty));
		EXPECT_EQ(sizeof(MosaicDefinitionTransaction) + 33 * sizeof(model::MosaicProperty) + 44, realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		MosaicDefinitionTransaction transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.PropertiesHeader.Count);
		test::SetMaxValue(transaction.MosaicNameSize);

		// Act:
		auto realSize = MosaicDefinitionTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(MosaicDefinitionTransaction) + 0xFF * sizeof(model::MosaicProperty) + 0xFF, realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
