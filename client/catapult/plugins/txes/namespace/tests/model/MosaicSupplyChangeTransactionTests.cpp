#include "src/model/MosaicSupplyChangeTransaction.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS MosaicSupplyChangeTransactionTests

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto expectedSize = baseSize // base
					+ sizeof(MosaicId) // mosaic id
					+ sizeof(uint8_t) // direction
					+ sizeof(Amount); // delta

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 17u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Mosaic_Supply_Change, static_cast<EntityType>(T::Entity_Type));
			EXPECT_EQ(2u, static_cast<uint8_t>(T::Current_Version));
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(MosaicSupplyChange);

	// endregion

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		MosaicSupplyChangeTransaction transaction;
		transaction.Size = 0;

		// Act:
		auto realSize = MosaicSupplyChangeTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(MosaicSupplyChangeTransaction), realSize);
	}
}}
