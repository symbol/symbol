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

#include "src/validators/Validators.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/utils/IntegerMath.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AggregateTransactionsHashValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AggregateTransactionsHash,)

	// region without transactions

	namespace {
		void AssertValidationWithNoEmbeddedTransactions(ValidationResult expectedResult, const Hash256& transactionsHash) {
			// Arrange:
			model::AggregateEmbeddedTransactionsNotification notification(transactionsHash, 0, nullptr);
			auto pValidator = CreateAggregateTransactionsHashValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenAggregateHasCorrectHash_NoEmbeddedTransactions) {
		AssertValidationWithNoEmbeddedTransactions(ValidationResult::Success, Hash256());
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasIncorrectHash_NoEmbeddedTransactions) {
		AssertValidationWithNoEmbeddedTransactions(Failure_Aggregate_Transactions_Hash_Mismatch, Hash256{ { 1 } });
	}

	// endregion

	// region with transactions

	namespace {
		Hash256 CalculateAggregateTransactionsHash(const std::vector<Hash256>& transactionHashes) {
			crypto::MerkleHashBuilder transactionsHashBuilder;
			for (const auto& transactionHash : transactionHashes)
				transactionsHashBuilder.update(transactionHash);

			Hash256 transactionsHash;
			transactionsHashBuilder.final(transactionsHash);
			return transactionsHash;
		}

		void AssertValidationWithEmbeddedTransactions(ValidationResult expectedResult, const consumer<Hash256&>& transactionsHashMutator) {
			// Arrange: generate random data for three variable sized transactions
			static constexpr auto Num_Transactions = 3u;
			uint32_t txBufferSize = 0;
			for (auto i = 0u; i < Num_Transactions; ++i) {
				uint32_t txSize = SizeOf32<model::EmbeddedTransaction>() + i + 1;
				txBufferSize += txSize + utils::GetPaddingSize(txSize, 8);
			}

			auto txBuffer = test::GenerateRandomVector(txBufferSize);

			// - correct sizes and calculate hashes
			size_t txOffset = 0;
			std::vector<Hash256> transactionHashes(Num_Transactions);
			for (auto i = 0u; i < Num_Transactions; ++i) {
				auto* pTransaction = reinterpret_cast<model::EmbeddedTransaction*>(&txBuffer[txOffset]);
				pTransaction->Size = SizeOf32<model::EmbeddedTransaction>() + i + 1;

				auto txHeaderSize = model::EmbeddedTransaction::Header_Size;
				crypto::Sha3_256(
						{ reinterpret_cast<const uint8_t*>(pTransaction) + txHeaderSize, pTransaction->Size - txHeaderSize },
						transactionHashes[i]);

				txOffset += pTransaction->Size + utils::GetPaddingSize(pTransaction->Size, 8);
			}

			// - calculate aggregate hash
			auto transactionsHash = CalculateAggregateTransactionsHash(transactionHashes);
			transactionsHashMutator(transactionsHash);

			// - create notification and validator
			auto* pTransactions = reinterpret_cast<const model::EmbeddedTransaction*>(txBuffer.data());
			model::AggregateEmbeddedTransactionsNotification notification(transactionsHash, Num_Transactions, pTransactions);
			auto pValidator = CreateAggregateTransactionsHashValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenAggregateHasCorrectHash_EmbeddedTransactions) {
		AssertValidationWithEmbeddedTransactions(ValidationResult::Success, [](const auto&) {});
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasIncorrectHash_EmbeddedTransactions) {
		AssertValidationWithEmbeddedTransactions(Failure_Aggregate_Transactions_Hash_Mismatch, [](auto& transactionsHash) {
			transactionsHash[Hash256::Size / 2] ^= 0xFF;
		});
	}

	// endregion
}}
