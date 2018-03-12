#include "src/model/SecretLockTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	using TransactionType = SecretLockTransaction;

#define TEST_CLASS SecretLockTransactionTests

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto expectedSize = baseSize // base
					+ sizeof(Mosaic) // mosaic
					+ sizeof(Height) // height
					+ sizeof(LockHashAlgorithm) // lock hash algorithm
					+ Hash512_Size // hash
					+ sizeof(Address); // address

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 114u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Secret_Lock, static_cast<EntityType>(T::Entity_Type));
			EXPECT_EQ(1u, static_cast<uint8_t>(T::Current_Version));
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(SecretLock);

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		SecretLockTransaction transaction;
		transaction.Size = 0;

		// Act:
		auto realSize = SecretLockTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(SecretLockTransaction), realSize);
	}

	// endregion
}}
