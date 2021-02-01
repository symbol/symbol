/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/model/FeeUtils.h"
#include "catapult/model/Transaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS FeeUtilsTests

	// region CalculateTransactionFee

	namespace {
		void AssertCanCalculateTransactionFee(uint32_t size, BlockFeeMultiplier multiplier, Amount expectedFee) {
			// Arrange:
			Transaction transaction;
			transaction.Size = size;

			// Act:
			auto fee = CalculateTransactionFee(multiplier, transaction);

			// Assert:
			EXPECT_EQ(expectedFee, fee) << "size = " << size << ", multiplier = " << multiplier;
		}
	}

	TEST(TEST_CLASS, CanCalculateTransactionFeeWhenFeeMultiplierIsZero) {
		AssertCanCalculateTransactionFee(123, BlockFeeMultiplier(0), Amount(0));
		AssertCanCalculateTransactionFee(842, BlockFeeMultiplier(0), Amount(0));
	}

	TEST(TEST_CLASS, CanCalculateTransactionFeeWhenFeeMultiplierIsNonzero) {
		AssertCanCalculateTransactionFee(123, BlockFeeMultiplier(4), Amount(123 * 4));
		AssertCanCalculateTransactionFee(842, BlockFeeMultiplier(11), Amount(842 * 11));
	}

	TEST(TEST_CLASS, CanCalculateTransactionFeeWhenFeeMultiplierIsNonzero_32BitOverflow) {
		AssertCanCalculateTransactionFee(842, BlockFeeMultiplier(15134406), Amount(842ull * 15134406));
	}

	// endregion

	// region CalculateTransactionMaxFeeMultiplier

	namespace {
		void AssertCanCalculateTransactionMaxFeeMultiplier(uint32_t size, Amount maxFee, BlockFeeMultiplier expectedMultiplier) {
			// Arrange:
			Transaction transaction;
			transaction.Size = size;
			transaction.MaxFee = maxFee;

			// Act:
			auto multiplier = CalculateTransactionMaxFeeMultiplier(transaction);

			// Assert:
			EXPECT_EQ(expectedMultiplier, multiplier) << "size = " << size << ", max fee = " << maxFee;
		}
	}

	TEST(TEST_CLASS, CanCalculateCalculateTransactionMaxFeeMultiplierWhenMaxFeeIsSizeMultiple) {
		AssertCanCalculateTransactionMaxFeeMultiplier(123, Amount(0), BlockFeeMultiplier(0));
		AssertCanCalculateTransactionMaxFeeMultiplier(222, Amount(222 * 3), BlockFeeMultiplier(3));
		AssertCanCalculateTransactionMaxFeeMultiplier(558, Amount(558 * 999), BlockFeeMultiplier(999));
	}

	TEST(TEST_CLASS, CanCalculateCalculateTransactionMaxFeeMultiplierWhenMaxFeeIsNotSizeMultiple) {
		AssertCanCalculateTransactionMaxFeeMultiplier(123, Amount(123 * 3 - 122), BlockFeeMultiplier(2));
		AssertCanCalculateTransactionMaxFeeMultiplier(222, Amount(222 * 3 - 50), BlockFeeMultiplier(2));
		AssertCanCalculateTransactionMaxFeeMultiplier(558, Amount(558 * 3 - 1), BlockFeeMultiplier(2));
		AssertCanCalculateTransactionMaxFeeMultiplier(558, Amount(558 * 3 + 1), BlockFeeMultiplier(3));
		AssertCanCalculateTransactionMaxFeeMultiplier(222, Amount(222 * 3 + 50), BlockFeeMultiplier(3));
		AssertCanCalculateTransactionMaxFeeMultiplier(123, Amount(123 * 3 + 122), BlockFeeMultiplier(3));
	}

	TEST(TEST_CLASS, CanCalculateCalculateTransactionMaxFeeMultiplierWhenMultiplierOverflowIsDetected) {
		AssertCanCalculateTransactionMaxFeeMultiplier(123, Amount(0xFFFF'FFFF'FFFF'FFFF), BlockFeeMultiplier(0xFFFF'FFFF));
		AssertCanCalculateTransactionMaxFeeMultiplier(558, Amount(0xFFFF'FFFF'FFFF'FF00), BlockFeeMultiplier(0xFFFF'FFFF));
	}

	// endregion
}}
