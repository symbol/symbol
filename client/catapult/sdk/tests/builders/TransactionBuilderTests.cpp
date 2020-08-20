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

		struct TransactionProperties {
		public:
			Amount MaxFee;
			Timestamp Deadline;
		};

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.MaxFee, transaction.MaxFee);
			EXPECT_EQ(expectedProperties.Deadline, transaction.Deadline);
		}

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
				auto* pData = pTransaction->DataPtr();
				std::iota(pData, pData + Additional_Data_Size, static_cast<uint8_t>(0));
				return pTransaction;
			}
		};

		void AssertCanBuildTransaction(const TransactionProperties& expectedProperties, const consumer<MockBuilder&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			MockBuilder builder(networkIdentifier, signer);
			buildTransaction(builder);
			auto pTransaction = builder.build();

			// Assert:
			EXPECT_EQ(signer, builder.signerPublicKey());
			ASSERT_EQ(sizeof(mocks::MockTransaction) + Additional_Data_Size, pTransaction->Size);
			EXPECT_EQ(Signature(), pTransaction->Signature);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(0xFFu, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
			EXPECT_EQ(static_cast<model::EntityType>(mocks::MockTransaction::Entity_Type), pTransaction->Type);

			AssertTransactionProperties(expectedProperties, *pTransaction);

			std::vector<uint8_t> expected(Additional_Data_Size);
			std::iota(expected.begin(), expected.end(), static_cast<uint8_t>(0));
			EXPECT_EQ_MEMORY(expected.data(), pTransaction->DataPtr(), expected.size());
		}
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateTransaction) {
		AssertCanBuildTransaction(TransactionProperties(), [](const auto&) {});
	}

	// endregion

	// region settings

	TEST(TEST_CLASS, CanSetMaxFee) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.MaxFee = Amount(12345);

		// Assert:
		AssertCanBuildTransaction(expectedProperties, [](auto& builder) {
			builder.setMaxFee(Amount(12345));
		});
	}

	TEST(TEST_CLASS, CanSetDeadline) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Deadline = Timestamp(54321);

		// Assert:
		AssertCanBuildTransaction(expectedProperties, [](auto& builder) {
			builder.setDeadline(Timestamp(54321));
		});
	}

	TEST(TEST_CLASS, CanSetMaxFeeAndDeadline) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.MaxFee = Amount(12345);
		expectedProperties.Deadline = Timestamp(54321);

		// Assert:
		AssertCanBuildTransaction(expectedProperties, [](auto& builder) {
			builder.setMaxFee(Amount(12345));
			builder.setDeadline(Timestamp(54321));
		});
	}

	// endregion
}}
