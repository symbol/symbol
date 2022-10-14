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

#include "src/validators/Validators.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/InvalidMerkleHashBuilder.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/utils/IntegerMath.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AggregateTransactionsHashValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AggregateTransactionsHash, {})

	// region without transactions - test utils

	namespace {
		constexpr auto Failure_Result = Failure_Aggregate_Transactions_Hash_Mismatch;

		void AssertValidationWithNoEmbeddedTransactions(
				ValidationResult expectedResult,
				uint8_t aggregateVersion,
				const Hash256& transactionsHash) {
			// Arrange:
			auto aggregateTransactionHash = test::GenerateRandomByteArray<Hash256>();
			model::AggregateEmbeddedTransactionsNotification notification(
					aggregateTransactionHash,
					aggregateVersion,
					transactionsHash,
					0,
					nullptr);
			auto pValidator = CreateAggregateTransactionsHashValidator({});

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// endregion

	// region without transactions - V2

	TEST(TEST_CLASS, SuccessWhenAggregateHasCorrectHash_NoEmbeddedTransactions_V2) {
		AssertValidationWithNoEmbeddedTransactions(ValidationResult::Success, 2, Hash256());
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasIncorrectHash_NoEmbeddedTransactions_V2) {
		AssertValidationWithNoEmbeddedTransactions(Failure_Result, 2, Hash256{ { 1 } });
	}

	// endregion

	// region without transactions - V1

	TEST(TEST_CLASS, SuccessWhenAggregateHasCorrectHash_NoEmbeddedTransactions_V1) {
		AssertValidationWithNoEmbeddedTransactions(ValidationResult::Success, 1, Hash256());
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasIncorrectHash_NoEmbeddedTransactions_V1) {
		AssertValidationWithNoEmbeddedTransactions(Failure_Result, 1, Hash256{ { 1 } });
	}

	// endregion

	// region with transactions - test utils

	namespace {
		template<typename TMerkleHashBuilder>
		Hash256 CalculateAggregateTransactionsHash(const std::vector<Hash256>& transactionHashes) {
			TMerkleHashBuilder transactionsHashBuilder;
			for (const auto& transactionHash : transactionHashes)
				transactionsHashBuilder.update(transactionHash);

			Hash256 transactionsHash;
			transactionsHashBuilder.final(transactionsHash);
			return transactionsHash;
		}

		void HashEmbeddedTransaction(const model::EmbeddedTransaction& transaction, Hash256& transactionHash) {
			crypto::Sha3_256_Builder builder;
			builder.update({ reinterpret_cast<const uint8_t*>(&transaction), transaction.Size });
			builder.final(transactionHash);
		}

		void HashEmbeddedTransactionWithPadding(const model::EmbeddedTransaction& transaction, Hash256& transactionHash) {
			auto paddingSize = utils::GetPaddingSize(transaction.Size, 8);
			crypto::Sha3_256_Builder builder;
			builder.update({ reinterpret_cast<const uint8_t*>(&transaction), transaction.Size });
			builder.update(std::vector<uint8_t>(paddingSize));
			builder.final(transactionHash);
		}

		std::vector<uint8_t> GenerateTransactionsBuffer(uint32_t numTransactions) {
			uint32_t txBufferSize = 0;
			for (auto i = 0u; i < numTransactions; ++i) {
				uint32_t txSize = SizeOf32<model::EmbeddedTransaction>() + i + 1;
				txBufferSize += txSize + utils::GetPaddingSize(txSize, 8);
			}

			auto txBuffer = test::GenerateRandomVector(txBufferSize);

			// - correct sizes and calculate hashes
			size_t txOffset = 0;
			for (auto i = 0u; i < numTransactions; ++i) {
				auto* pTransaction = reinterpret_cast<model::EmbeddedTransaction*>(&txBuffer[txOffset]);
				pTransaction->Size = SizeOf32<model::EmbeddedTransaction>() + i + 1;

				auto paddingSize = utils::GetPaddingSize(pTransaction->Size, 8);
				txOffset += pTransaction->Size + paddingSize;
			}

			return txBuffer;
		}

		void AssertValidationWithEmbeddedTransactions(
				ValidationResult expectedResult,
				uint8_t aggregateVersion,
				const consumer<const model::EmbeddedTransaction&, Hash256&>& transactionHasher,
				const std::function<Hash256 (const std::vector<Hash256>&)>& aggregateTransactionsHashCalculator,
				const consumer<Hash256&>& transactionsHashMutator = [](const auto&) {}) {
			// Arrange: generate random data for three variable sized transactions
			static constexpr auto Num_Transactions = 3u;
			auto txBuffer = GenerateTransactionsBuffer(Num_Transactions);

			// - calculate hashes
			size_t txOffset = 0;
			std::vector<Hash256> transactionHashes(Num_Transactions);
			for (auto i = 0u; i < Num_Transactions; ++i) {
				auto* pTransaction = reinterpret_cast<model::EmbeddedTransaction*>(&txBuffer[txOffset]);
				transactionHasher(*pTransaction, transactionHashes[i]);
				txOffset += pTransaction->Size + utils::GetPaddingSize(pTransaction->Size, 8);
			}

			// - calculate aggregate hash
			auto transactionsHash = aggregateTransactionsHashCalculator(transactionHashes);
			transactionsHashMutator(transactionsHash);

			// - create notification and validator
			auto aggregateTransactionHash = test::GenerateRandomByteArray<Hash256>();
			auto* pTransactions = reinterpret_cast<const model::EmbeddedTransaction*>(txBuffer.data());
			model::AggregateEmbeddedTransactionsNotification notification(
					aggregateTransactionHash,
					aggregateVersion,
					transactionsHash,
					Num_Transactions,
					pTransactions);
			auto pValidator = CreateAggregateTransactionsHashValidator({});

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		void AssertValidationWithEmbeddedTransactionsKnownCorrupt(
				ValidationResult expectedResult,
				uint8_t aggregateVersion,
				const consumer<Hash256&>& transactionsHashMutator = [](const auto&) {}) {
			// Arrange: generate random data for three variable sized transactions
			static constexpr auto Num_Transactions = 3u;
			auto txBuffer = GenerateTransactionsBuffer(Num_Transactions);
			auto transactionHashes = test::GenerateRandomDataVector<Hash256>(Num_Transactions);

			// - generate random aggregate hash
			auto transactionsHash = test::GenerateRandomByteArray<Hash256>();
			auto knownCorruptTransactionsHash = transactionsHash;
			transactionsHashMutator(transactionsHash);

			// - create notification and validator
			auto aggregateTransactionHash = test::GenerateRandomByteArray<Hash256>();
			auto* pTransactions = reinterpret_cast<const model::EmbeddedTransaction*>(txBuffer.data());
			model::AggregateEmbeddedTransactionsNotification notification(
					aggregateTransactionHash,
					aggregateVersion,
					transactionsHash,
					Num_Transactions,
					pTransactions);
			auto pValidator = CreateAggregateTransactionsHashValidator({
				{ test::GenerateRandomByteArray<Hash256>(), test::GenerateRandomByteArray<Hash256>() },
				{ aggregateTransactionHash, knownCorruptTransactionsHash },
				{ test::GenerateRandomByteArray<Hash256>(), test::GenerateRandomByteArray<Hash256>() }
			});

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// endregion

	// region with transactions - V2

	TEST(TEST_CLASS, SuccessWhenAggregateHasValidMerkleUnpaddedHash_EmbeddedTransactions_V2) {
		AssertValidationWithEmbeddedTransactions(
				ValidationResult::Success,
				2,
				HashEmbeddedTransaction,
				CalculateAggregateTransactionsHash<crypto::MerkleHashBuilder>);
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasCorruptHash_EmbeddedTransactions_V2) {
		AssertValidationWithEmbeddedTransactions(
				Failure_Result,
				2,
				HashEmbeddedTransaction,
				CalculateAggregateTransactionsHash<crypto::MerkleHashBuilder>,
				[](auto& transactionsHash) {
					transactionsHash[Hash256::Size / 2] ^= 0xFF;
				});
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasValidMerklePaddedHash_EmbeddedTransactions_V2) {
		AssertValidationWithEmbeddedTransactions(
				Failure_Result,
				2,
				HashEmbeddedTransactionWithPadding,
				CalculateAggregateTransactionsHash<crypto::MerkleHashBuilder>);
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasInvalidMerkleUnpaddedHash_EmbeddedTransactions_V2) {
		AssertValidationWithEmbeddedTransactions(
				Failure_Result,
				2,
				HashEmbeddedTransaction,
				CalculateAggregateTransactionsHash<crypto::InvalidMerkleHashBuilder>);
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasInvalidMerklePaddedHash_EmbeddedTransactions_V2) {
		AssertValidationWithEmbeddedTransactions(
				Failure_Result,
				2,
				HashEmbeddedTransactionWithPadding,
				CalculateAggregateTransactionsHash<crypto::InvalidMerkleHashBuilder>);
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasMatchingKnownCorruptHash_EmbeddedTransactions_V2) {
		AssertValidationWithEmbeddedTransactionsKnownCorrupt(Failure_Result, 2);
	}

	// endregion

	// region with transactions - V1

	TEST(TEST_CLASS, SuccessWhenAggregateHasValidMerkleUnpaddedHash_EmbeddedTransactions_V1) {
		AssertValidationWithEmbeddedTransactions(
				ValidationResult::Success,
				1,
				HashEmbeddedTransaction,
				CalculateAggregateTransactionsHash<crypto::MerkleHashBuilder>);
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasCorruptHash_EmbeddedTransactions_V1) {
		AssertValidationWithEmbeddedTransactions(
				Failure_Result,
				1,
				HashEmbeddedTransaction,
				CalculateAggregateTransactionsHash<crypto::MerkleHashBuilder>,
				[](auto& transactionsHash) {
					transactionsHash[Hash256::Size / 2] ^= 0xFF;
				});
	}

	TEST(TEST_CLASS, SuccessWhenAggregateHasValidMerklePaddedHash_EmbeddedTransactions_V1) {
		AssertValidationWithEmbeddedTransactions(
				ValidationResult::Success,
				1,
				HashEmbeddedTransactionWithPadding,
				CalculateAggregateTransactionsHash<crypto::MerkleHashBuilder>);
	}

	TEST(TEST_CLASS, SuccessWhenAggregateHasInvalidMerkleUnpaddedHash_EmbeddedTransactions_V1) {
		AssertValidationWithEmbeddedTransactions(
				ValidationResult::Success,
				1,
				HashEmbeddedTransaction,
				CalculateAggregateTransactionsHash<crypto::InvalidMerkleHashBuilder>);
	}

	TEST(TEST_CLASS, SuccessWhenAggregateHasInvalidMerklePaddedHash_EmbeddedTransactions_V1) {
		AssertValidationWithEmbeddedTransactions(
				ValidationResult::Success,
				1,
				HashEmbeddedTransactionWithPadding,
				CalculateAggregateTransactionsHash<crypto::InvalidMerkleHashBuilder>);
	}

	TEST(TEST_CLASS, SuccessWhenAggregateHasMatchingKnownCorruptHash_EmbeddedTransactions_V1) {
		AssertValidationWithEmbeddedTransactionsKnownCorrupt(ValidationResult::Success, 1);
	}

	TEST(TEST_CLASS, FailureWhenAggregateHasMismatchingKnownCorruptHash_EmbeddedTransactions_V1) {
		AssertValidationWithEmbeddedTransactionsKnownCorrupt(Failure_Result, 1, [](auto& transactionsHash) {
			transactionsHash[Hash256::Size / 2] ^= 0xFF;
		});
	}

	// endregion
}}
