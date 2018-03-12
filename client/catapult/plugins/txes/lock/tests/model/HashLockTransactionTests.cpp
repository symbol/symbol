#include "src/model/HashLockTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	using TransactionType = HashLockTransaction;

#define TEST_CLASS HashLockTransactionTests

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto expectedSize = baseSize // base
					+ sizeof(Mosaic) // mosaic
					+ sizeof(Height) // height
					+ Hash256_Size; // hash

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 56u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Hash_Lock, static_cast<EntityType>(T::Entity_Type));
			EXPECT_EQ(1u, static_cast<uint8_t>(T::Current_Version));
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(HashLock);

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		HashLockTransaction transaction;
		transaction.Size = 0;

		// Act:
		auto realSize = HashLockTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(HashLockTransaction), realSize);
	}

	// endregion
}}
