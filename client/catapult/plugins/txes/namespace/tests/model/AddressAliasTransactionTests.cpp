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

#include "src/model/AddressAliasTransaction.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS AddressAliasTransactionTests

	using TransactionType = AddressAliasTransaction;

	// region size + properties

	namespace {
		template<typename T>
		void AssertEntityHasExpectedSize(size_t baseSize) {
			// Arrange:
			auto expectedSize =
					baseSize // base
					+ sizeof(uint8_t) // alias action
					+ sizeof(NamespaceId) // link
					+ sizeof(Address); // address alias

			// Assert:
			EXPECT_EQ(expectedSize, sizeof(T));
			EXPECT_EQ(baseSize + 34u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(Entity_Type_Alias_Address, T::Entity_Type);
			EXPECT_EQ(1u, T::Current_Version);
		}
	}

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(AddressAlias)

	// endregion

	// region CalculateRealSize

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		TransactionType transaction;
		transaction.Size = 0;

		// Act:
		auto realSize = TransactionType::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(TransactionType), realSize);
	}

	// endregion
}}
