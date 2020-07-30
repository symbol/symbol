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

#include "src/model/NamespaceRegistrationTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NamespaceRegistrationTransactionTests

	// region size + alignment + properties

#define TRANSACTION_FIELDS FIELD(ParentId) FIELD(Id) FIELD(RegistrationType) FIELD(NameSize)

	namespace {
		template<typename T>
		void AssertTransactionHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto expectedSize = baseSize;

#define FIELD(X) expectedSize += SizeOf32<decltype(T::X)>();
			TRANSACTION_FIELDS
#undef FIELD

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 18u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasProperAlignment() {
#define FIELD(X) EXPECT_ALIGNED(T, X);
			TRANSACTION_FIELDS
#undef FIELD

			// ParentId and Duration perfectly overlap as part of union
			EXPECT_ALIGNED(T, Duration);
			EXPECT_EQ(sizeof(T::ParentId), sizeof(T::Duration));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Namespace_Registration, T::Entity_Type);
			EXPECT_EQ(1u, T::Current_Version);
		}
	}

#undef TRANSACTION_FIELDS

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(NamespaceRegistration)

	// endregion

	// region data pointers

	namespace {
		std::unique_ptr<NamespaceRegistrationTransaction> GenerateNamespaceRegistrationWithName(uint8_t nameSize) {
			uint32_t entitySize = SizeOf32<NamespaceRegistrationTransaction>() + nameSize;
			auto pTransaction = utils::MakeUniqueWithSize<NamespaceRegistrationTransaction>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->NameSize = nameSize;
			return pTransaction;
		}

		struct NamespaceRegistrationTransactionTraits {
			static auto GenerateEntityWithAttachments(uint16_t count) {
				return GenerateNamespaceRegistrationWithName(static_cast<uint8_t>(count * 10));
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.NamePtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, NamespaceRegistrationTransactionTraits) // NamePtr

	// endregion

	// region IsRoot / IsChild

	namespace {
		std::unique_ptr<NamespaceRegistrationTransaction> GenerateNamespaceRegistrationWithRegistrationType(
				NamespaceRegistrationType registrationType) {
			auto pTransaction = GenerateNamespaceRegistrationWithName(10);
			pTransaction->RegistrationType = registrationType;
			return pTransaction;
		}
	}

	TEST(TEST_CLASS, IsRootIsChildPredicatesReturnCorrectValuesForRoot) {
		// Arrange:
		auto pTransaction = GenerateNamespaceRegistrationWithRegistrationType(NamespaceRegistrationType::Root);

		// Act + Assert:
		EXPECT_TRUE(pTransaction->IsRootRegistration());
		EXPECT_FALSE(pTransaction->IsChildRegistration());
	}

	TEST(TEST_CLASS, IsRootIsChildPredicatesReturnCorrectValuesForChild) {
		// Arrange:
		auto pTransaction = GenerateNamespaceRegistrationWithRegistrationType(NamespaceRegistrationType::Child);

		// Act + Assert:
		EXPECT_FALSE(pTransaction->IsRootRegistration());
		EXPECT_TRUE(pTransaction->IsChildRegistration());
	}

	TEST(TEST_CLASS, IsRootIsChildPredicatesReturnCorrectValuesForOther) {
		// Arrange:
		auto pTransaction = GenerateNamespaceRegistrationWithRegistrationType(static_cast<NamespaceRegistrationType>(0xFF));

		// Act + Assert:
		EXPECT_FALSE(pTransaction->IsRootRegistration());
		EXPECT_FALSE(pTransaction->IsChildRegistration());
	}

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		NamespaceRegistrationTransaction transaction;
		transaction.Size = 0;
		transaction.NameSize = 100;

		// Act:
		auto realSize = NamespaceRegistrationTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(NamespaceRegistrationTransaction) + 100, realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		NamespaceRegistrationTransaction transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.NameSize);

		// Act:
		auto realSize = NamespaceRegistrationTransaction::CalculateRealSize(transaction);

		// Assert:
		ASSERT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(NamespaceRegistrationTransaction) + 0xFF, realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
