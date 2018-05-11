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

#include "src/builders/TransactionBuilder.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"
#include <numeric>

namespace catapult { namespace builders {

#define TEST_CLASS TransactionBuilderTests

	namespace {
		constexpr auto Additional_Data_Size = 123;

		class MockBuilder : public TransactionBuilder {
		public:
			MockBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
					: TransactionBuilder(networkIdentifier, signer)
			{}

		public:
			std::unique_ptr<mocks::MockTransaction> build() const {
				auto pTransaction = createTransaction<mocks::MockTransaction>(sizeof(mocks::MockTransaction) + Additional_Data_Size);

				// 1. set sizes upfront, so that pointers are calculated correctly
				pTransaction->Data.Size = Additional_Data_Size;

				// 2. set data
				auto pData = pTransaction->DataPtr();
				std::iota(pData, pData + Additional_Data_Size, static_cast<uint8_t>(0));
				return pTransaction;
			}
		};

		void AssertCanBuildTransaction(
				const consumer<MockBuilder&>& buildTransaction,
				const consumer<const model::Transaction&>& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			MockBuilder builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = builder.build();

			// Assert:
			ASSERT_EQ(sizeof(mocks::MockTransaction) + Additional_Data_Size, pTransaction->Size);
			EXPECT_EQ(Signature{}, pTransaction->Signature);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x62FF, pTransaction->Version);
			EXPECT_EQ(static_cast<model::EntityType>(mocks::MockTransaction::Entity_Type), pTransaction->Type);

			validateTransaction(*pTransaction);

			std::vector<uint8_t> expected(Additional_Data_Size);
			std::iota(expected.begin(), expected.end(), static_cast<uint8_t>(0));
			EXPECT_TRUE(0 == std::memcmp(expected.data(), pTransaction->DataPtr(), expected.size()));
		}

		auto CreatePropertyChecker(Amount fee, Timestamp deadline) {
			return [fee, deadline](const auto& transaction) {
				EXPECT_EQ(fee, transaction.Fee);
				EXPECT_EQ(deadline, transaction.Deadline);
			};
		}
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateTransaction) {
		// Assert:
		AssertCanBuildTransaction(
				[](const auto&) {},
				CreatePropertyChecker(Amount(0), Timestamp(0)));
	}

	// endregion

	// region settings

	TEST(TEST_CLASS, CanSetFee) {
		// Assert:
		AssertCanBuildTransaction(
				[](auto& builder) {
					builder.setFee(Amount(12345));
				},
				CreatePropertyChecker(Amount(12345), Timestamp(0)));
	}

	TEST(TEST_CLASS, CanSetDeadline) {
		// Assert:
		AssertCanBuildTransaction(
				[](auto& builder) {
					builder.setDeadline(Timestamp(54321));
				},
				CreatePropertyChecker(Amount(0), Timestamp(54321)));
	}

	TEST(TEST_CLASS, CanSetFeeAndDeadline) {
		// Assert:
		AssertCanBuildTransaction(
				[](auto& builder) {
					builder.setFee(Amount(12345));
					builder.setDeadline(Timestamp(54321));
				},
				CreatePropertyChecker(Amount(12345), Timestamp(54321)));
	}

	// endregion
}}
