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

#include "catapult/model/BlockUtils.h"
#include "sdk/src/extensions/BlockExtensions.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS BlockUtilsTests

	// region hashes - CalculateBlockTransactionsHash

	namespace {
		struct CalculateBlockTransactionsHashTestContext {
		public:
			explicit CalculateBlockTransactionsHashTestContext(size_t numTransactions) {
				// generate random transactions
				crypto::MerkleHashBuilder builder;
				TransactionInfos.reserve(numTransactions);
				for (auto i = 0u; i < numTransactions; ++i) {
					auto entityHash = test::GenerateRandomByteArray<Hash256>();
					auto merkleComponentHash = test::GenerateRandomByteArray<Hash256>();
					TransactionInfos.emplace_back(test::GenerateRandomTransaction(), entityHash);
					TransactionInfos.back().MerkleComponentHash = merkleComponentHash;
					TransactionInfoPointers.push_back(&TransactionInfos.back());

					// calculate the expected block transactions hash
					builder.update(merkleComponentHash);
				}

				builder.final(ExpectedBlockTransactionsHash);
			}

		public:
			std::vector<TransactionInfo> TransactionInfos;
			std::vector<const TransactionInfo*> TransactionInfoPointers;
			Hash256 ExpectedBlockTransactionsHash;
		};
	}

	TEST(TEST_CLASS, CanCalculateBlockTransactionsHash) {
		// Arrange:
		CalculateBlockTransactionsHashTestContext context(5);

		// Act:
		Hash256 actualBlockTransactionsHash;
		CalculateBlockTransactionsHash(context.TransactionInfoPointers, actualBlockTransactionsHash);

		// Assert:
		EXPECT_EQ(context.ExpectedBlockTransactionsHash, actualBlockTransactionsHash);
	}

	namespace {
		using CalculateBlockTransactionsHashTestContextModifierFunc = consumer<CalculateBlockTransactionsHashTestContext&>;

		void AssertSignificantChange(size_t numHashes, const CalculateBlockTransactionsHashTestContextModifierFunc& modifier) {
			// Arrange:
			CalculateBlockTransactionsHashTestContext context(numHashes);

			Hash256 blockTransactionsHash1;
			CalculateBlockTransactionsHash(context.TransactionInfoPointers, blockTransactionsHash1);

			// Act: change the transaction info pointers
			modifier(context);

			// - recalculate the merkle hash
			Hash256 blockTransactionsHash2;
			CalculateBlockTransactionsHash(context.TransactionInfoPointers, blockTransactionsHash2);

			// Assert:
			EXPECT_NE(blockTransactionsHash1, blockTransactionsHash2);
		}
	}

	TEST(TEST_CLASS, BlockTransactionsHashChangesWhenAnyTransactionMerkleComponentHashChanges) {
		AssertSignificantChange(5, [](auto& context) { context.TransactionInfos[2].MerkleComponentHash[0] ^= 0xFF; });
	}

	TEST(TEST_CLASS, BlockTransactionsHashChangesWhenTransactionOrderChanges) {
		AssertSignificantChange(5, [](auto& context) {
			std::swap(context.TransactionInfoPointers[1], context.TransactionInfoPointers[2]);
		});
	}

	namespace {
		void AssertInsignificantChange(size_t numHashes, const CalculateBlockTransactionsHashTestContextModifierFunc& modifier) {
			// Arrange:
			CalculateBlockTransactionsHashTestContext context(numHashes);

			Hash256 blockTransactionsHash1;
			CalculateBlockTransactionsHash(context.TransactionInfoPointers, blockTransactionsHash1);

			// Act: change the transaction info pointers
			modifier(context);

			// - recalculate the merkle hash
			Hash256 blockTransactionsHash2;
			CalculateBlockTransactionsHash(context.TransactionInfoPointers, blockTransactionsHash2);

			// Assert:
			EXPECT_EQ(blockTransactionsHash1, blockTransactionsHash2);
		}
	}

	TEST(TEST_CLASS, BlockTransactionsHashDoesNotChangeWhenAnyTransactionEntityHashChanges) {
		AssertInsignificantChange(5, [](auto& context) { context.TransactionInfos[2].EntityHash[0] ^= 0xFF; });
	}

	TEST(TEST_CLASS, CanCalculateBlockTransactionsHash_Deterministic) {
		// Arrange:
		auto seedHashes = {
			utils::ParseByteArray<Hash256>("36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8"),
			utils::ParseByteArray<Hash256>("8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28"),
			utils::ParseByteArray<Hash256>("6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7"),
			utils::ParseByteArray<Hash256>("2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681"),
			utils::ParseByteArray<Hash256>("421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE")
		};

		std::vector<TransactionInfo> transactionInfos;
		transactionInfos.reserve(seedHashes.size());
		std::vector<const TransactionInfo*> transactionInfoPointers;
		for (const auto& seedHash : seedHashes) {
			// - notice that only MerkleComponentHash should be used in calculation
			transactionInfos.emplace_back(test::GenerateRandomTransaction(), test::GenerateRandomByteArray<Hash256>());
			transactionInfos.back().MerkleComponentHash = seedHash;
			transactionInfoPointers.push_back(&transactionInfos.back());
		}

		// Act:
		Hash256 actualBlockTransactionsHash;
		CalculateBlockTransactionsHash(transactionInfoPointers, actualBlockTransactionsHash);

		// Assert:
		auto expectedHash = utils::ParseByteArray<Hash256>("DEFB4BF7ACF2145500087A02C88F8D1FCF27B8DEF4E0FDABE09413D87A3F0D09");
		EXPECT_EQ(expectedHash, actualBlockTransactionsHash);
	}

	// endregion

	// region hashes - CalculateGenerationHash

	TEST(TEST_CLASS, GenerationHashIsCalculatedAsExpected) {
		// Arrange:
		auto gamma = utils::ParseByteArray<crypto::ProofGamma>("6FB9C930C0AC6BEF09D6DFEBD091AE83C91B35F2C0305B05B4F6F7AF4B6FC1F0");

		// Act:
		auto hash = CalculateGenerationHash(gamma);

		// Assert:
		auto expectedHash = utils::ParseByteArray<GenerationHash>("9620004E86866D24F8881E77ED6F8464FAE1DF53F1194FF3FF1A87DC80EF22E8");
		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region sign / verify

	namespace {
		auto CreateSignedBlock(size_t numTransactions) {
			// random generation hash is used because VerifyBlockHeaderSignature should succeed independent of generation hash
			auto signer = test::GenerateKeyPair();
			auto pBlock = test::GenerateBlockWithTransactions(signer, test::GenerateRandomTransactions(numTransactions));
			extensions::BlockExtensions(test::GenerateRandomByteArray<GenerationHashSeed>()).updateBlockTransactionsHash(*pBlock);
			SignBlockHeader(signer, *pBlock);
			return pBlock;
		}
	}

	TEST(TEST_CLASS, CanSignAndVerifyBlockWithoutTransactions) {
		// Arrange:
		auto pBlock = CreateSignedBlock(0);

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	TEST(TEST_CLASS, CanSignAndVerifyBlockWithTransactions) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	TEST(TEST_CLASS, CanSignAndVerifyBlockHeaderWithTransactions) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);
		pBlock->Size = sizeof(BlockHeader);

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert: the block header should still verify even though it doesn't have transaction data
		EXPECT_TRUE(isVerified);
	}

	TEST(TEST_CLASS, CannotVerifyBlockWithAlteredSignature) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);
		pBlock->Signature[0] ^= 0xFF;

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	TEST(TEST_CLASS, CannotVerifyBlockWithAlteredData) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);
		pBlock->Timestamp = pBlock->Timestamp + Timestamp(1);

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	TEST(TEST_CLASS, CannotVerifyBlockWithAlteredBlockTransactionsHash) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);
		pBlock->TransactionsHash[0] ^= 0xFF;

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	TEST(TEST_CLASS, CanVerifyBlockWithAlteredTransaction) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);
		auto transactions = pBlock->Transactions();
		auto iter = transactions.begin();
		++iter;
		iter->Deadline = iter->Deadline + Timestamp(1);

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert: the header should verify but the block transactions hash should not match
		EXPECT_TRUE(isVerified);
	}

	// endregion

	// region fees - CalculateBlockTransactionsInfo

	TEST(TEST_CLASS, CanCalculateBlockTransactionsInfoForBlockWithZeroTransactions) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		pBlock->FeeMultiplier = BlockFeeMultiplier(3);

		// Act:
		auto blockTransactionsInfo = CalculateBlockTransactionsInfo(*pBlock);

		// Assert:
		EXPECT_EQ(0u, blockTransactionsInfo.Count);
		EXPECT_EQ(Amount(), blockTransactionsInfo.TotalFee);
	}

	TEST(TEST_CLASS, CanCalculateBlockTransactionsInfoForBlockWithSingleTransaction) {
		// Arrange:
		auto pBlock = test::GenerateBlockWithTransactions(test::ConstTransactions{ test::GenerateRandomTransactionWithSize(132) });
		pBlock->FeeMultiplier = BlockFeeMultiplier(3);

		// Act:
		auto blockTransactionsInfo = CalculateBlockTransactionsInfo(*pBlock);

		// Assert:
		EXPECT_EQ(1u, blockTransactionsInfo.Count);
		EXPECT_EQ(Amount(3 * 132), blockTransactionsInfo.TotalFee);
	}

	TEST(TEST_CLASS, CanCalculateBlockTransactionsInfoForBlockWithMultipleTransactions) {
		// Arrange:
		auto pBlock = test::GenerateBlockWithTransactions(test::ConstTransactions{
			test::GenerateRandomTransactionWithSize(132),
			test::GenerateRandomTransactionWithSize(222),
			test::GenerateRandomTransactionWithSize(552)
		});
		pBlock->FeeMultiplier = BlockFeeMultiplier(3);

		// Act:
		auto blockTransactionsInfo = CalculateBlockTransactionsInfo(*pBlock);

		// Assert:
		EXPECT_EQ(3u, blockTransactionsInfo.Count);
		EXPECT_EQ(Amount(3 * 906), blockTransactionsInfo.TotalFee);
	}

	// endregion

	// region create block - PreviousBlockContext

	TEST(TEST_CLASS, CanCreateDefaultPreviousBlockContext) {
		// Act:
		PreviousBlockContext context;

		// Assert:
		EXPECT_EQ(Hash256(), context.BlockHash);
		EXPECT_EQ(GenerationHash(), context.GenerationHash);
		EXPECT_EQ(Height(0), context.BlockHeight);
		EXPECT_EQ(Timestamp(0), context.Timestamp);
	}

	TEST(TEST_CLASS, CanCreatePreviousBlockContextFromBlockElement) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		pBlock->Height = Height(123);
		pBlock->Timestamp = Timestamp(9876);

		auto blockHash = test::GenerateRandomByteArray<Hash256>();
		auto generationHash = test::GenerateRandomByteArray<GenerationHash>();
		auto blockElement = test::BlockToBlockElement(*pBlock, blockHash);
		blockElement.GenerationHash = generationHash;

		// Act:
		PreviousBlockContext context(blockElement);

		// Assert:
		EXPECT_EQ(blockHash, context.BlockHash);
		EXPECT_EQ(generationHash, context.GenerationHash);
		EXPECT_EQ(Height(123), context.BlockHeight);
		EXPECT_EQ(Timestamp(9876), context.Timestamp);
	}

	// endregion

	// region create block - CreateBlock

	namespace {
		struct SharedPointerTraits {
			using ContainerType = Transactions;

			static ContainerType MapTransactions(test::MutableTransactions& transactions) {
				ContainerType container;
				for (const auto& pTransaction : transactions)
					container.push_back(pTransaction);

				return container;
			}
		};

		size_t SumTransactionSizes(const Transactions& transactions, bool includePadding = false) {
			auto transactionsSize = 0u;
			auto lastPaddingSize = 0u;
			for (const auto& pTransaction : transactions) {
				transactionsSize += pTransaction->Size;

				if (includePadding) {
					lastPaddingSize = utils::GetPaddingSize(pTransaction->Size, 8);
					transactionsSize += lastPaddingSize;
				}
			}

			return transactionsSize - lastPaddingSize;
		}

		void AssertTransactionsInBlock(const Block& block, const Transactions& expectedTransactions) {
			auto transactionCount = 0u;
			auto transactionsSize = 0u;

			// 1. iterate over block transactions and compare against expected
			auto expectedIter = expectedTransactions.cbegin();
			for (const auto& blockTransaction : block.Transactions()) {
				ASSERT_NE(expectedTransactions.cend(), expectedIter);
				EXPECT_EQ(**expectedIter++, blockTransaction);
				transactionsSize += blockTransaction.Size;
				++transactionCount;
			}

			// 2. check iterated size and count
			EXPECT_EQ(expectedTransactions.size(), transactionCount);
			EXPECT_EQ(SumTransactionSizes(expectedTransactions), transactionsSize);

			// 3. check padding bytes
			std::vector<uint8_t> transactionPaddingBytes;
			const auto* pTransactionBytes = reinterpret_cast<const uint8_t*>(block.TransactionsPtr());
			for (auto i = 0u; i < expectedTransactions.size(); ++i) {
				const auto& pExpectedTransaction = expectedTransactions[i];
				pTransactionBytes += pExpectedTransaction->Size;

				if (i < expectedTransactions.size() - 1) {
					auto paddingSize = utils::GetPaddingSize(pExpectedTransaction->Size, 8);
					transactionPaddingBytes.insert(transactionPaddingBytes.end(), pTransactionBytes, pTransactionBytes + paddingSize);
					pTransactionBytes += paddingSize;
				}
			}

			EXPECT_EQ(SumTransactionSizes(expectedTransactions, true), transactionsSize + transactionPaddingBytes.size());
			EXPECT_EQ(std::vector<uint8_t>(transactionPaddingBytes.size(), 0), transactionPaddingBytes);
		}

		template<typename TContainerTraits>
		void AssertCanCreateBlock(size_t numTransactions) {
			// Arrange:
			auto signer = test::GenerateKeyPair();

			PreviousBlockContext context;
			context.BlockHeight = Height(1234);
			context.BlockHash = test::GenerateRandomByteArray<Hash256>();
			context.GenerationHash = test::GenerateRandomByteArray<GenerationHash>();

			auto randomTransactions = test::GenerateRandomTransactions(numTransactions);
			auto transactions = TContainerTraits::MapTransactions(randomTransactions);

			// Act:
			auto pBlock = CreateBlock(context, static_cast<NetworkIdentifier>(0x17), signer.publicKey(), transactions);

			// Assert:
			ASSERT_EQ(sizeof(BlockHeader) + SumTransactionSizes(transactions, true), pBlock->Size);
			EXPECT_EQ(Signature(), pBlock->Signature);

			EXPECT_EQ(signer.publicKey(), pBlock->SignerPublicKey);
			EXPECT_EQ(Block::Current_Version, pBlock->Version);
			EXPECT_EQ(static_cast<NetworkIdentifier>(0x17), pBlock->Network);
			EXPECT_EQ(Entity_Type_Block, pBlock->Type);

			EXPECT_EQ(Height(1235), pBlock->Height);
			EXPECT_EQ(Timestamp(), pBlock->Timestamp);
			EXPECT_EQ(Difficulty(), pBlock->Difficulty);
			EXPECT_EQ(BlockFeeMultiplier(), pBlock->FeeMultiplier);
			EXPECT_EQ(context.BlockHash, pBlock->PreviousBlockHash);
			EXPECT_EQ(Hash256(), pBlock->TransactionsHash);
			EXPECT_EQ(Hash256(), pBlock->ReceiptsHash);
			EXPECT_EQ(Hash256(), pBlock->StateHash);
			EXPECT_EQ(GetSignerAddress(*pBlock), pBlock->BeneficiaryAddress);

			AssertTransactionsInBlock(*pBlock, transactions);
		}
	}

	TEST(TEST_CLASS, CanCreateBlockWithoutTransactions) {
		AssertCanCreateBlock<SharedPointerTraits>(0);
	}

	TEST(TEST_CLASS, CanCreateBlockWithTransactions) {
		AssertCanCreateBlock<SharedPointerTraits>(5);
	}

	// endregion

	// region create block - StitchBlock

	namespace {
		template<typename TContainerTraits>
		void AssertCanStitchBlock(size_t numTransactions) {
			// Arrange:
			BlockHeader blockHeader;
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&blockHeader), sizeof(BlockHeader) });

			auto randomTransactions = test::GenerateRandomTransactions(numTransactions);
			auto transactions = TContainerTraits::MapTransactions(randomTransactions);

			// Act:
			auto pBlock = StitchBlock(blockHeader, transactions);

			// Assert:
			ASSERT_EQ(sizeof(BlockHeader) + SumTransactionSizes(transactions, true), pBlock->Size);

			EXPECT_EQ_MEMORY(
					reinterpret_cast<const uint8_t*>(&blockHeader) + sizeof(BlockHeader::Size),
					reinterpret_cast<const uint8_t*>(pBlock.get()) + sizeof(BlockHeader::Size),
					sizeof(BlockHeader) - sizeof(BlockHeader::Size));

			AssertTransactionsInBlock(*pBlock, transactions);
		}
	}

	TEST(TEST_CLASS, CanStitchBlockWithoutTransactions) {
		AssertCanStitchBlock<SharedPointerTraits>(0);
	}

	TEST(TEST_CLASS, CanStitchBlockWithTransactions) {
		AssertCanStitchBlock<SharedPointerTraits>(5);
	}

	// endregion
}}
