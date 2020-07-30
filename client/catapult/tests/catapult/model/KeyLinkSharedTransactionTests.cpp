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

#include "catapult/model/KeyLinkSharedTransaction.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS KeyLinkSharedTransactionTests

#pragma pack(push, 1)

	namespace {
		// use KeyLinkSharedTransactionBody class name so that all tests are in same test suite (KeyLinkSharedTransactionTests)
		// use a non-key type (MosaicId) in tests
		template<typename THeader>
		struct KeyLinkSharedTransactionBody : public BasicKeyLinkTransactionBody<THeader, MosaicId, static_cast<EntityType>(17)> {};

		DEFINE_EMBEDDABLE_TRANSACTION(KeyLinkShared)
	}

#pragma pack(pop)

	// region size + alignment + properties

#define TRANSACTION_FIELDS FIELD(LinkedPublicKey) FIELD(LinkAction)

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
			EXPECT_EQ(baseSize + 9u, sizeof(T));
		}

		template<typename T>
		void AssertTransactionHasProperAlignment() {
#define FIELD(X) EXPECT_ALIGNED(T, X);
			TRANSACTION_FIELDS
#undef FIELD
		}

		template<typename T>
		void AssertTransactionHasExpectedProperties() {
			// Assert:
			EXPECT_EQ(static_cast<EntityType>(17), T::Entity_Type);
			EXPECT_EQ(1u, T::Current_Version);
		}
	}

#undef TRANSACTION_FIELDS

	ADD_BASIC_TRANSACTION_SIZE_PROPERTY_TESTS(KeyLinkShared)

	// endregion

	TEST(TEST_CLASS, CanCalculateRealSizeWithReasonableValues) {
		// Arrange:
		KeyLinkSharedTransaction transaction;
		transaction.Size = 0;

		// Act:
		auto realSize = KeyLinkSharedTransaction::CalculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(KeyLinkSharedTransaction), realSize);
	}
}}
