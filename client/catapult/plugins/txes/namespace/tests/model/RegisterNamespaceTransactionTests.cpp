#include "src/model/RegisterNamespaceTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS RegisterNamespaceTransactionTests

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto expectedSize = baseSize // base
					+ sizeof(NamespaceType) // namespace type
					+ sizeof(NamespaceId) // parent id or duration
					+ sizeof(NamespaceId) // id
					+ sizeof(uint8_t); // namespace name size

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 18u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Register_Namespace, static_cast<EntityType>(T::Entity_Type));
			EXPECT_EQ(2u, static_cast<uint8_t>(T::Current_Version));
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(RegisterNamespace);

	// endregion

	// region data pointers

	namespace {
		std::unique_ptr<RegisterNamespaceTransaction> GenerateRegisterNamespaceWithName(uint8_t nameSize) {
			uint32_t entitySize = sizeof(RegisterNamespaceTransaction) + nameSize;
			auto pTransaction = utils::MakeUniqueWithSize<RegisterNamespaceTransaction>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->NamespaceNameSize = nameSize;
			return pTransaction;
		}

		struct RegisterNamespaceTransactionTraits {
			static auto GenerateEntityWithAttachments(uint16_t count) {
				return GenerateRegisterNamespaceWithName(static_cast<uint8_t>(count * 10));
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.NamePtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, RegisterNamespaceTransactionTraits) // NamePtr

	// endregion

	// region IsRoot / IsChild

	namespace {
		std::unique_ptr<RegisterNamespaceTransaction> GenerateRegisterNamespaceWithNamespaceType(NamespaceType namespaceType) {
			auto pTransaction = GenerateRegisterNamespaceWithName(10);
			pTransaction->NamespaceType = namespaceType;
			return pTransaction;
		}
	}

	TEST(TEST_CLASS, IsRootIsChildPredicatesReturnCorrectValuesForRoot) {
		// Arrange:
		auto pTransaction = GenerateRegisterNamespaceWithNamespaceType(NamespaceType::Root);

		// Act + Assert:
		EXPECT_TRUE(pTransaction->IsRootRegistration());
		EXPECT_FALSE(pTransaction->IsChildRegistration());
	}

	TEST(TEST_CLASS, IsRootIsChildPredicatesReturnCorrectValuesForChild) {
		// Arrange:
		auto pTransaction = GenerateRegisterNamespaceWithNamespaceType(NamespaceType::Child);

		// Act + Assert:
		EXPECT_FALSE(pTransaction->IsRootRegistration());
		EXPECT_TRUE(pTransaction->IsChildRegistration());
	}

	TEST(TEST_CLASS, IsRootIsChildPredicatesReturnCorrectValuesForOther) {
		// Arrange:
		auto pTransaction = GenerateRegisterNamespaceWithNamespaceType(static_cast<NamespaceType>(0xFF));

		// Act + Assert:
		EXPECT_FALSE(pTransaction->IsRootRegistration());
		EXPECT_FALSE(pTransaction->IsChildRegistration());
	}

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		RegisterNamespaceTransaction transaction;
		transaction.Size = 0;
		transaction.NamespaceNameSize = 100;

		// Act:
		auto realSize = RegisterNamespaceTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(RegisterNamespaceTransaction) + 100, realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		RegisterNamespaceTransaction transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.NamespaceNameSize);

		// Act:
		auto realSize = RegisterNamespaceTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(RegisterNamespaceTransaction) + 0xFF, realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
