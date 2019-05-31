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

#include "src/extensions/BlockExtensions.h"
#include "src/extensions/TransactionExtensions.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransactionPluginWithCustomBuffers.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS BlockExtensionsTests

	namespace {
		struct TransactionHashes {
			Hash256 EntityHash;
			Hash256 MerkleComponentHash;
		};

		GenerationHash GetNetworkGenerationHash() {
			return utils::ParseByteArray<GenerationHash>("CE076EF4ABFBC65B046987429E274EC31506D173E91BF102F16BEB7FB8176230");
		}

		struct BasicTraits {
		public:
			template<typename TAction>
			static void RunExtensionsTest(TAction action) {
				// Act:
				action(BlockExtensions(GetNetworkGenerationHash()));
			}

			static Hash256 CalculateExpectedBlockTransactionsHash(const model::Block& block) {
				// calculate the expected block transactions hash
				crypto::MerkleHashBuilder builder;
				for (const auto& transaction : block.Transactions())
					builder.update(model::CalculateHash(transaction, GetNetworkGenerationHash()));

				Hash256 expectedBlockTransactionsHash;
				builder.final(expectedBlockTransactionsHash);
				return expectedBlockTransactionsHash;
			}

			static TransactionHashes CalculateTransactionHashes(const model::Transaction& transaction) {
				TransactionHashes hashes;
				hashes.EntityHash = model::CalculateHash(transaction, GetNetworkGenerationHash());
				hashes.MerkleComponentHash = hashes.EntityHash;
				return hashes;
			}
		};

		struct RegistryTraits {
		public:
			template<typename TAction>
			static void RunExtensionsTest(TAction action) {
				// Arrange:
				auto transactionRegistry = CreateCustomTransactionRegistry();

				// Act:
				action(BlockExtensions(GetNetworkGenerationHash(), transactionRegistry));
			}

			static Hash256 CalculateExpectedBlockTransactionsHash(const model::Block& block) {
				// calculate the expected block transactions hash
				auto transactionRegistry = CreateCustomTransactionRegistry();
				crypto::MerkleHashBuilder builder;
				for (const auto& transaction : block.Transactions()) {
					auto transactionElement = model::TransactionElement(transaction);
					model::UpdateHashes(transactionRegistry, GetNetworkGenerationHash(), transactionElement);
					builder.update(transactionElement.MerkleComponentHash);
				}

				Hash256 expectedBlockTransactionsHash;
				builder.final(expectedBlockTransactionsHash);
				return expectedBlockTransactionsHash;
			}

			static TransactionHashes CalculateTransactionHashes(const model::Transaction& transaction) {
				auto transactionRegistry = CreateCustomTransactionRegistry();
				const auto& plugin = *transactionRegistry.findPlugin(transaction.Type);

				TransactionHashes hashes;
				hashes.EntityHash = model::CalculateHash(transaction, GetNetworkGenerationHash(), plugin.dataBuffer(transaction));
				hashes.MerkleComponentHash = model::CalculateMerkleComponentHash(transaction, hashes.EntityHash, transactionRegistry);
				return hashes;
			}

		private:
			static model::TransactionRegistry CreateCustomTransactionRegistry() {
				// create a custom transaction registry
				auto pPlugin = mocks::CreateMockTransactionPluginWithCustomBuffers(
						mocks::OffsetRange{ 6, 10 },
						std::vector<mocks::OffsetRange>{ { 7, 11 }, { 4, 7 }, { 12, 20 } });
				auto registry = model::TransactionRegistry();
				registry.registerPlugin(std::move(pPlugin));
				return registry;
			}
		};
	}

#define REGISTRY_DEPENDENT_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_WithoutRegistry) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BasicTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_WithRegistry) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegistryTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		constexpr auto Default_Num_Transactions = 5u;

		template<typename TTraits>
		auto CreateValidBlock(const crypto::KeyPair& signer, size_t numTransactions = Default_Num_Transactions) {
			// generate transactions
			test::ConstTransactions transactions;
			for (auto i = 1u; i <= numTransactions; ++i) {
				auto pTransaction = test::GenerateRandomTransaction(GetNetworkGenerationHash());
				transactions.push_back(std::move(pTransaction));
			}

			// create block and sign it using the specified extensions
			auto pBlock = test::GenerateBlockWithTransactions(signer, transactions);
			TTraits::RunExtensionsTest([&signer, &block = *pBlock](const auto& extensions) {
				extensions.signFullBlock(signer, block);
			});

			return pBlock;
		}

		template<typename TTraits>
		auto CreateValidBlock(size_t numTransactions = Default_Num_Transactions) {
			return CreateValidBlock<TTraits>(test::GenerateKeyPair(), numTransactions);
		}

		auto GenerateBlockWithTransactions() {
			return test::GenerateBlockWithTransactions(7, Height(7));
		}
	}

	// region Block Transactions Hash Extensions

	namespace {
		template<typename TTraits, typename THashCalculator>
		void AssertBlockTransactionsHashCalculation(THashCalculator blockTransactionsHashCalculator) {
			// Arrange: generate a random block and calculate the expected block transactions hash
			auto pBlock = GenerateBlockWithTransactions();
			pBlock->BlockTransactionsHash = Hash256();
			auto expectedBlockTransactionsHash = TTraits::CalculateExpectedBlockTransactionsHash(*pBlock);

			// Act: calculate the actual block transactions hash
			auto blockTransactionsHash = blockTransactionsHashCalculator(*pBlock);

			// Assert: the actual hash is nonzero and matches expected hash
			EXPECT_NE(Hash256(), blockTransactionsHash);
			EXPECT_EQ(expectedBlockTransactionsHash, blockTransactionsHash);
		}
	}

	REGISTRY_DEPENDENT_TEST(CanUpdateBlockTransactionsHash) {
		// Arrange:
		TTraits::RunExtensionsTest([](const auto& extensions) {
			// Assert:
			AssertBlockTransactionsHashCalculation<TTraits>([&extensions](auto& block) {
				// Act:
				extensions.updateBlockTransactionsHash(block);
				return block.BlockTransactionsHash;
			});
		});
	}

	REGISTRY_DEPENDENT_TEST(CanCalculateBlockTransactionsHash) {
		// Arrange:
		TTraits::RunExtensionsTest([](const auto& extensions) {
			// Assert:
			AssertBlockTransactionsHashCalculation<TTraits>([&extensions](const auto& block) {
				// Act:
				Hash256 blockTransactionsHash;
				extensions.calculateBlockTransactionsHash(block, blockTransactionsHash);

				// Sanity: the block wasn't modified
				EXPECT_EQ(Hash256(), block.BlockTransactionsHash);
				return blockTransactionsHash;
			});
		});
	}

	// endregion

	// region SignFullBlock

	REGISTRY_DEPENDENT_TEST(SignFullBlockProducesVerifiableBlock) {
		// Arrange:
		TTraits::RunExtensionsTest([](const auto& extensions) {
			// - create a block and clear the signature and transactions hash
			auto signer = test::GenerateKeyPair();
			auto pBlock = CreateValidBlock<TTraits>(signer);
			pBlock->Signature = {};
			pBlock->BlockTransactionsHash = {};

			// Sanity: the block does not verify
			EXPECT_NE(VerifyFullBlockResult::Success, extensions.verifyFullBlock(*pBlock));

			// Act: sign the block and then verify it
			extensions.signFullBlock(signer, *pBlock);
			auto result = extensions.verifyFullBlock(*pBlock);

			// Assert: fields have been updated and it is verifiable
			EXPECT_NE(Signature(), pBlock->Signature);
			EXPECT_NE(Hash256(), pBlock->BlockTransactionsHash);
			EXPECT_EQ(VerifyFullBlockResult::Success, result);
		});
	}

	// endregion

	// region VerifyFullBlock

	REGISTRY_DEPENDENT_TEST(VerifyFullBlockSucceedsWhenVerifyingValidBlock) {
		// Arrange:
		TTraits::RunExtensionsTest([](const auto& extensions) {
			auto pBlock = CreateValidBlock<TTraits>();

			// Act:
			auto result = extensions.verifyFullBlock(*pBlock);

			// Assert:
			EXPECT_EQ(VerifyFullBlockResult::Success, result);
		});
	}

	REGISTRY_DEPENDENT_TEST(VerifyFullBlockFailsWhenBlockDataIsAltered) {
		// Arrange:
		TTraits::RunExtensionsTest([](const auto& extensions) {
			auto pBlock = CreateValidBlock<TTraits>();
			pBlock->Timestamp = pBlock->Timestamp + Timestamp(1);

			// Act:
			auto result = extensions.verifyFullBlock(*pBlock);

			// Assert:
			EXPECT_EQ(VerifyFullBlockResult::Invalid_Block_Signature, result);
		});
	}

	REGISTRY_DEPENDENT_TEST(VerifyFullBlockFailsWhenTransactionDataIsAltered) {
		// Arrange:
		TTraits::RunExtensionsTest([](const auto& extensions) {
			auto signer = test::GenerateKeyPair();
			auto pBlock = CreateValidBlock<TTraits>(signer);
			auto transactions = pBlock->Transactions();
			auto iter = ++transactions.begin();
			auto& transaction = reinterpret_cast<model::Transaction&>(*iter);
			transaction.Deadline = transaction.Deadline + Timestamp(1);
			extensions.signFullBlock(signer, *pBlock); // fix block transactions hash and block signature

			// Act:
			auto result = extensions.verifyFullBlock(*pBlock);

			// Assert:
			EXPECT_EQ(VerifyFullBlockResult::Invalid_Transaction_Signature, result);
		});
	}

	REGISTRY_DEPENDENT_TEST(VerifyFullBlockFailsWhenBlockTransactionsHashIsAltered) {
		// Arrange:
		TTraits::RunExtensionsTest([](const auto& extensions) {
			auto signer = test::GenerateKeyPair();
			auto pBlock = CreateValidBlock<TTraits>(signer);
			pBlock->BlockTransactionsHash[0] ^= 0xFF;
			model::SignBlockHeader(signer, *pBlock); // fix block signature

			// Act:
			auto result = extensions.verifyFullBlock(*pBlock);

			// Assert:
			EXPECT_EQ(VerifyFullBlockResult::Invalid_Block_Transactions_Hash, result);
		});
	}

	REGISTRY_DEPENDENT_TEST(VerifyFullBlockFailsWhenGenerationHashIsAltered) {
		// Arrange:
		TTraits::RunExtensionsTest([](const auto& extensions) {
			auto pBlock = CreateValidBlock<TTraits>();

			// Sanity:
			EXPECT_EQ(VerifyFullBlockResult::Success, extensions.verifyFullBlock(*pBlock));

			// Act:
			auto result = BlockExtensions(test::GenerateRandomByteArray<GenerationHash>()).verifyFullBlock(*pBlock);

			// Assert:
			EXPECT_EQ(VerifyFullBlockResult::Invalid_Block_Transactions_Hash, result);
		});
	}

	// endregion

	// region ConvertBlockToBlockElement

	namespace {
		model::BlockElement AssertBlockToBlockElementConversionExcludingTransactions(
				const BlockExtensions& extensions,
				const model::Block& block) {
			// Arrange:
			auto generationHash = test::GenerateRandomByteArray<GenerationHash>();

			// Act:
			auto element = extensions.convertBlockToBlockElement(block, generationHash);

			// Assert: the block element refers to the original block
			EXPECT_EQ(block, element.Block);
			EXPECT_EQ(model::CalculateHash(block), element.EntityHash);
			EXPECT_EQ(generationHash, element.GenerationHash);
			return element;
		}
	}

	REGISTRY_DEPENDENT_TEST(CanConvertBlockToBlockElement_WithoutTransactions) {
		// Arrange:
		TTraits::RunExtensionsTest([](const auto& extensions) {
			// - create a block with no transactions
			auto pBlock = CreateValidBlock<TTraits>(0);

			// Act + Assert:
			auto element = AssertBlockToBlockElementConversionExcludingTransactions(extensions, *pBlock);

			// - no transactions are present
			EXPECT_TRUE(element.Transactions.empty());
		});
	}

	REGISTRY_DEPENDENT_TEST(CanConvertBlockToBlockElement_WithTransactions) {
		// Arrange:
		TTraits::RunExtensionsTest([](const auto& extensions) {
			// - create a block with transactions
			constexpr auto Num_Transactions = 7u;
			auto pBlock = CreateValidBlock<TTraits>(Num_Transactions);

			// Act + Assert:
			auto element = AssertBlockToBlockElementConversionExcludingTransactions(extensions, *pBlock);

			// - all transactions refer to transactions in the original block
			auto i = 0u;
			ASSERT_EQ(Num_Transactions, element.Transactions.size());
			for (const auto& transaction : pBlock->Transactions()) {
				const auto message = "tx at " + std::to_string(i);
				const auto& transactionElement = element.Transactions[i];
				auto expectedHashes = TTraits::CalculateTransactionHashes(transaction);

				EXPECT_EQ(transaction, transactionElement.Transaction) << message;
				EXPECT_EQ(expectedHashes.EntityHash, transactionElement.EntityHash) << message;
				EXPECT_EQ(expectedHashes.MerkleComponentHash, transactionElement.MerkleComponentHash) << message;
				++i;
			}

			EXPECT_EQ(Num_Transactions, i);
		});
	}

	// endregion

	// region Deterministic Entity Sanity

	TEST(TEST_CLASS, DeterministicBlockIsFullyVerifiable) {
		// Arrange:
		auto generationHash = utils::ParseByteArray<GenerationHash>(test::Deterministic_Network_Generation_Hash_String);
		auto pBlock = test::GenerateDeterministicBlock();

		// Act: deterministic block does not contain any aggregate transactions, so no transaction registry is required
		auto result = BlockExtensions(generationHash).verifyFullBlock(*pBlock);

		// Assert:
		EXPECT_EQ(VerifyFullBlockResult::Success, result);
	}

	// endregion
}}
