#include "catapult/model/BlockUtils.h"
#include "sdk/src/extensions/BlockExtensions.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/MerkleHashBuilder.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	//region hashes

	// region CalculateBlockTransactionsHash

	namespace {
		struct CalculateBlockTransactionsHashTestContext {
		public:
			explicit CalculateBlockTransactionsHashTestContext(size_t numTransactions) {
				// generate random transactions
				crypto::MerkleHashBuilder builder;
				TransactionInfos.reserve(numTransactions);
				for (auto i = 0u; i < numTransactions; ++i) {
					auto entityHash = test::GenerateRandomData<Hash256_Size>();
					auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();
					TransactionInfos.emplace_back(test::GenerateRandomTransaction(), entityHash, merkleComponentHash);
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

	TEST(BlockUtilsTests, CanCalculateBlockTransactionsHash) {
		// Arrange:
		CalculateBlockTransactionsHashTestContext context(5);

		// Act:
		Hash256 actualBlockTransactionsHash;
		CalculateBlockTransactionsHash(context.TransactionInfoPointers, actualBlockTransactionsHash);

		// Assert:
		EXPECT_EQ(test::ToHexString(context.ExpectedBlockTransactionsHash), test::ToHexString(actualBlockTransactionsHash));
	}

	namespace {
		using CalculateBlockTransactionsHashTestContextModifierFunc = std::function<void (CalculateBlockTransactionsHashTestContext&)>;

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
			EXPECT_NE(test::ToHexString(blockTransactionsHash1), test::ToHexString(blockTransactionsHash2));
		}
	}

	TEST(BlockUtilsTests, BlockTransactionsHashChangesIfAnyTransactionMerkleComponentHashChanges) {
		// Assert:
		AssertSignificantChange(5, [](auto& context) { context.TransactionInfos[2].MerkleComponentHash[0] ^= 0xFF; });
	}

	TEST(BlockUtilsTests, BlockTransactionsHashChangesIfTransactionOrderChanges) {
		// Assert:
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
			EXPECT_EQ(test::ToHexString(blockTransactionsHash1), test::ToHexString(blockTransactionsHash2));
		}
	}

	TEST(BlockUtilsTests, BlockTransactionsHashDoesNotChangeIfAnyTransactionEntityHashChanges) {
		// Assert:
		AssertInsignificantChange(5, [](auto& context) { context.TransactionInfos[2].EntityHash[0] ^= 0xFF; });
	}

	TEST(BlockUtilsTests, CanCalculateBlockTransactionsHash_Deterministic) {
		// Arrange:
		auto seedHashes = {
			test::ToArray<Hash256_Size>("36C8213162CDBC78767CF43D4E06DDBE0D3367B6CEAEAEB577A50E2052441BC8"),
			test::ToArray<Hash256_Size>("8A316E48F35CDADD3F827663F7535E840289A16A43E7134B053A86773E474C28"),
			test::ToArray<Hash256_Size>("6D80E71F00DFB73B358B772AD453AEB652AE347D3E098AE269005A88DA0B84A7"),
			test::ToArray<Hash256_Size>("2AE2CA59B5BB29721BFB79FE113929B6E52891CAA29CBF562EBEDC46903FF681"),
			test::ToArray<Hash256_Size>("421D6B68A6DF8BB1D5C9ACF7ED44515E77945D42A491BECE68DA009B551EE6CE")
		};

		std::vector<TransactionInfo> infos;
		infos.reserve(seedHashes.size());
		std::vector<const TransactionInfo*> transactionInfoPointers;
		for (const auto& seedHash : seedHashes) {
			// - notice that only MerkleComponentHash should be used in calculation
			infos.emplace_back(test::GenerateRandomTransaction(), test::GenerateRandomData<Hash256_Size>(), seedHash);
			transactionInfoPointers.push_back(&infos.back());
		}

		// Act:
		Hash256 actualBlockTransactionsHash;
		CalculateBlockTransactionsHash(transactionInfoPointers, actualBlockTransactionsHash);

		// Assert:
		EXPECT_EQ("DEFB4BF7ACF2145500087A02C88F8D1FCF27B8DEF4E0FDABE09413D87A3F0D09", test::ToHexString(actualBlockTransactionsHash));
	}

	// endregion

	// region CalculateGenerationHash

	TEST(BlockUtilsTests, GenerationHashIsCalculatedAsExpected) {
		// Arrange:
		auto previousGenerationHash = test::ToArray<Hash256_Size>("57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6");
		auto keyPair = crypto::KeyPair::FromString("A41BE076B942D915EA3330B135D35C5A959A2DCC50BBB393C6407984D4A3B564");

		// Act:
		auto hash = CalculateGenerationHash(previousGenerationHash, keyPair.publicKey());

		// Assert:
		EXPECT_EQ("575E4F520DC2C026F1C9021FD3773F236F0872A03B4AEFC22A9E0066FF204A23", test::ToHexString(hash));
	}

	// endregion

	// region sign / verify

	namespace {
		auto CreateSignedBlock(size_t numTransactions) {
			auto signer = test::GenerateKeyPair();
			auto pBlock = test::GenerateBlockWithTransactions(signer, test::GenerateRandomTransactions(numTransactions));
			extensions::UpdateBlockTransactionsHash(*pBlock);
			SignBlockHeader(signer, *pBlock);
			return pBlock;
		}
	}

	TEST(BlockUtilsTests, CanSignAndVerifyBlockWithoutTransactions) {
		// Arrange:
		auto pBlock = CreateSignedBlock(0);

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	TEST(BlockUtilsTests, CanSignAndVerifyBlockWithTransactions) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert:
		EXPECT_TRUE(isVerified);
	}

	TEST(BlockUtilsTests, CanSignAndVerifyBlockHeaderWithTransactions) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);
		pBlock->Size = sizeof(model::Block);

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert: the block header should still verify even though it doesn't have transaction data
		EXPECT_TRUE(isVerified);
	}

	TEST(BlockUtilsTests, CannotVerifyBlockWithAlteredSignature) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);
		pBlock->Signature[0] ^= 0xFF;

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	TEST(BlockUtilsTests, CannotVerifyBlockWithAlteredData) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);
		pBlock->Timestamp = pBlock->Timestamp + Timestamp(1);

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	TEST(BlockUtilsTests, CannotVerifyBlockWithAlteredBlockTransactionsHash) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);
		pBlock->BlockTransactionsHash[0] ^= 0xFF;

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert:
		EXPECT_FALSE(isVerified);
	}

	TEST(BlockUtilsTests, CanVerifyBlockWithAlteredTransaction) {
		// Arrange:
		auto pBlock = CreateSignedBlock(3);
		auto it = pBlock->Transactions().begin();
		++it;
		it->Deadline = it->Deadline + Timestamp(1);

		// Act:
		auto isVerified = VerifyBlockHeaderSignature(*pBlock);

		// Assert: the header should verify but the block transactions hash should not match
		EXPECT_TRUE(isVerified);
	}

	// endregion

	// region PreviousBlockContext

	TEST(BlockUtilsTests, CanCreateDefaultPreviousBlockContext) {
		// Act:
		PreviousBlockContext context;

		// Assert:
		EXPECT_EQ(Hash256{}, context.BlockHash);
		EXPECT_EQ(Hash256{}, context.GenerationHash);
		EXPECT_EQ(Height(0), context.BlockHeight);
		EXPECT_EQ(Timestamp(0), context.Timestamp);
	}

	TEST(BlockUtilsTests, CanCreatePreviousBlockContextFromBlockElement) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		pBlock->Height = Height(123);
		pBlock->Timestamp = Timestamp(9876);

		auto blockHash = test::GenerateRandomData<Hash256_Size>();
		auto generationHash = test::GenerateRandomData<Hash256_Size>();
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

	// region create

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

		template<typename TContainerTraits>
		void AssertBlockSetsProperFields(size_t numTransactions) {
			// Arrange:
			auto signer = test::GenerateKeyPair();

			PreviousBlockContext context;
			context.BlockHeight = Height(1234);
			context.BlockHash = test::GenerateRandomData<Hash256_Size>();
			context.GenerationHash = test::GenerateRandomData<Hash256_Size>();

			auto randomTransactions = test::GenerateRandomTransactions(numTransactions);
			auto transactions = TContainerTraits::MapTransactions(randomTransactions);

			// Act:
			auto pBlock = CreateBlock(context, static_cast<NetworkIdentifier>(0x17), signer.publicKey(), transactions);

			// Assert: the only reason for static_casts here is to solve gcc's linking problem
			const auto& block = *pBlock;
			EXPECT_EQ(EntityType::Block, block.Type);
			EXPECT_EQ(static_cast<uint8_t>(0x17), block.Network());
			EXPECT_EQ(static_cast<uint8_t>(Block::Current_Version), block.EntityVersion());
			EXPECT_EQ(signer.publicKey(), block.Signer);
			EXPECT_EQ(Signature{}, block.Signature);
			EXPECT_EQ(Timestamp(), block.Timestamp);
			EXPECT_EQ(Height(1235), block.Height);
			EXPECT_EQ(Difficulty(), block.Difficulty);
			EXPECT_EQ(context.BlockHash, block.PreviousBlockHash);

			auto transactionCount = 0u;
			size_t blockSize = sizeof(Block);
			auto iter = transactions.cbegin();
			for (const auto& blockTransaction : block.Transactions()) {
				ASSERT_NE(transactions.cend(), iter);
				EXPECT_EQ(**iter++, blockTransaction);
				blockSize += blockTransaction.Size;
				++transactionCount;
			}

			EXPECT_EQ(numTransactions, transactionCount);
			EXPECT_EQ(blockSize, block.Size);
		}
	}

	TEST(BlockUtilsTests, CreateBlockSetsProperFields_WithoutTransactions_SharedPointer) {
		// Assert:
		AssertBlockSetsProperFields<SharedPointerTraits>(0);
	}

	TEST(BlockUtilsTests, CreateBlockSetsProperFields_WithTransactions_SharedPointer) {
		// Assert:
		AssertBlockSetsProperFields<SharedPointerTraits>(5);
	}

	// endregion
}}
