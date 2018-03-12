#include "src/model/SecretProofTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	using TransactionType = SecretProofTransaction;

#define TEST_CLASS SecretProofTransactionTests

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto expectedSize = baseSize // base
					+ sizeof(uint8_t) // lock hash algorithm
					+ Hash512_Size // hash size
					+ sizeof(uint16_t); // proof size

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 67u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Secret_Proof, static_cast<EntityType>(T::Entity_Type));
			EXPECT_EQ(1u, static_cast<uint8_t>(T::Current_Version));
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(SecretProof);

	// endregion

	// region data pointers

	namespace {
		struct SecretProofTransactionTraits {
			static auto GenerateEntityWithAttachments(uint16_t proofSize) {
				uint32_t entitySize = sizeof(TransactionType) + proofSize;
				auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
				pTransaction->Size = entitySize;
				pTransaction->ProofSize = proofSize;
				return pTransaction;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.ProofPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, SecretProofTransactionTraits);

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		TransactionType transaction;
		transaction.Size = 0;
		transaction.ProofSize = 100;

		// Act:
		auto realSize = TransactionType::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(TransactionType) + 100, realSize);
	}

	TEST(TEST_CLASS, CalculateRealSizeDoesNotOverflowWithMaxValues) {
		// Arrange:
		TransactionType transaction;
		test::SetMaxValue(transaction.Size);
		test::SetMaxValue(transaction.ProofSize);

		// Act:
		auto realSize = TransactionType::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(0xFFFFFFFF, transaction.Size);
		EXPECT_EQ(sizeof(TransactionType) + 0xFFFF, realSize);
		EXPECT_GT(0xFFFFFFFF, realSize);
	}

	// endregion
}}
