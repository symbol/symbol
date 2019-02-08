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
			auto expectedPropertiesHeaderSize =
					sizeof(uint8_t) // count
					+ sizeof(uint8_t) // flags
					+ sizeof(uint8_t); // divisibility
			auto expectedSize =
					baseSize // base
					+ sizeof(uint32_t) // nonce
					+ sizeof(MosaicId) // id
					+ expectedPropertiesHeaderSize;

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 15u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Mosaic_Definition, static_cast<EntityType>(T::Entity_Type));
			EXPECT_EQ(3u, static_cast<uint8_t>(T::Current_Version));
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(MosaicDefinition)

	// endregion

	// region data pointers

	namespace {
		struct MosaicDefinitionTransactionTraits {
			static auto GenerateEntityWithAttachments(uint8_t propertiesCount) {
				uint32_t entitySize = sizeof(MosaicDefinitionTransaction) + propertiesCount * sizeof(MosaicProperty);
				auto pTransaction = utils::MakeUniqueWithSize<MosaicDefinitionTransaction>(entitySize);
				pTransaction->Size = entitySize;
				pTransaction->PropertiesHeader.Count = propertiesCount;
				return pTransaction;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.PropertiesPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, MosaicDefinitionTransactionTraits)

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		MosaicDefinitionTransaction transaction;
		transaction.Size = 0;
		transaction.PropertiesHeader.Count = 33;

		// Act:
		auto realSize = MosaicDefinitionTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(9u, sizeof(MosaicProperty));
		EXPECT_EQ(sizeof(MosaicDefinitionTransaction) + 33 * sizeof(MosaicProperty), realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		MosaicDefinitionTransaction transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.PropertiesHeader.Count);

		// Act:
		auto realSize = MosaicDefinitionTransaction::CalculateRealSize(transaction);

		// Assert:
		ASSERT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(MosaicDefinitionTransaction) + 0xFF * sizeof(MosaicProperty), realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
