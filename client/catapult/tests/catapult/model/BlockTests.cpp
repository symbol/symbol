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

#include "catapult/model/Block.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/SizePrefixedEntityContainerTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/Alignment.h"

namespace catapult { namespace model {

#define TEST_CLASS BlockTests

	// region size + alignment - BlockHeader / Block

#define BLOCK_HEADER_FIELDS \
	FIELD(Height) \
	FIELD(Timestamp) \
	FIELD(Difficulty) \
	FIELD(GenerationHashProof) \
	FIELD(PreviousBlockHash) \
	FIELD(TransactionsHash) \
	FIELD(ReceiptsHash) \
	FIELD(StateHash) \
	FIELD(BeneficiaryAddress) \
	FIELD(FeeMultiplier)

	TEST(TEST_CLASS, BlockHeaderHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(VerifiableEntity);

#define FIELD(X) expectedSize += SizeOf32<decltype(BlockHeader::X)>();
		BLOCK_HEADER_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(BlockHeader));
		EXPECT_EQ(112u + 260, sizeof(BlockHeader));
	}

	TEST(TEST_CLASS, BlockHeaderHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(BlockHeader, X);
		BLOCK_HEADER_FIELDS
#undef FIELD

		EXPECT_EQ(0u, (sizeof(BlockHeader) + sizeof(PaddedBlockFooter)) % 4);
	}

#undef BLOCK_HEADER_FIELDS

	TEST(TEST_CLASS, BlockHasExpectedSize) {
		using BlockAlias = Block; // use alias to bypass lint rule
		EXPECT_EQ(sizeof(BlockHeader), sizeof(BlockAlias));
	}

	// endregion

	// region size + alignment - PaddedBlockFooter

	TEST(TEST_CLASS, PaddedBlockFooterHasExpectedSize) {
		// Arrange:
		auto expectedSize = 4u;

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(PaddedBlockFooter));
		EXPECT_EQ(4u, sizeof(PaddedBlockFooter));
	}

	TEST(TEST_CLASS, PaddedBlockFooterHasProperAlignment) {
		EXPECT_EQ(0u, (sizeof(BlockHeader) + sizeof(PaddedBlockFooter)) % 8);
	}

	// endregion

	// region size + alignment - ImportanceBlockFooter

#define IMPORTANCE_BLOCK_FOOTER_FIELDS \
	FIELD(VotingEligibleAccountsCount) \
	FIELD(HarvestingEligibleAccountsCount) \
	FIELD(TotalVotingBalance) \
	FIELD(PreviousImportanceBlockHash)

	TEST(TEST_CLASS, ImportanceBlockFooterHasExpectedSize) {
		// Arrange:
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(ImportanceBlockFooter::X)>();
		IMPORTANCE_BLOCK_FOOTER_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(ImportanceBlockFooter));
		EXPECT_EQ(52u, sizeof(ImportanceBlockFooter));
	}

	TEST(TEST_CLASS, ImportanceBlockFooterHasProperAlignment) {
		struct AlignedImportanceBlockHeader : public BlockHeader, public ImportanceBlockFooter {};

#define FIELD(X) EXPECT_ALIGNED(AlignedImportanceBlockHeader, X);
		IMPORTANCE_BLOCK_FOOTER_FIELDS
#undef FIELD

		EXPECT_EQ(0u, (sizeof(BlockHeader) + sizeof(ImportanceBlockFooter)) % 8);
	}

#undef IMPORTANCE_BLOCK_FOOTER_FIELDS

	// endregion

	// region IsImportanceBlock / GetBlockHeaderSize / GetBlockHeaderDataBuffer / GetTransactionPayloadSize

	namespace {
		void AssertBlockHeaderDataBuffer(uint32_t expectedDataBufferSize, EntityType type, uint8_t version) {
			// Arrange:
			Block block;
			block.Size = GetBlockHeaderSize(type, version) + 123;
			block.Version = version;
			block.Type = type;

			// Act:
			auto buffer = GetBlockHeaderDataBuffer(block);

			// Assert:
			EXPECT_EQ(reinterpret_cast<uint8_t*>(&block) + Block::Header_Size, buffer.pData) << type;
			EXPECT_EQ(expectedDataBufferSize, buffer.Size) << type;
		}

		void AssertTransactionPayloadSize(EntityType type, uint8_t version) {
			// Arrange:
			Block block;
			block.Size = GetBlockHeaderSize(type, version) + 123;
			block.Version = version;
			block.Type = type;

			// Act:
			auto payloadSize = GetTransactionPayloadSize(block);

			// Assert:
			EXPECT_EQ(123u, payloadSize) << type;
		}
	}

	TEST(TEST_CLASS, IsImportanceBlock_ReturnsTrueOnlyForImportanceBlocks) {
		EXPECT_FALSE(IsImportanceBlock(Entity_Type_Block_Nemesis, 1));
		EXPECT_FALSE(IsImportanceBlock(Entity_Type_Block_Importance, 1));
		EXPECT_FALSE(IsImportanceBlock(static_cast<EntityType>(0), 1));
		EXPECT_FALSE(IsImportanceBlock(Entity_Type_Block_Normal, 1));

		EXPECT_TRUE(IsImportanceBlock(Entity_Type_Block_Nemesis, 2));
		EXPECT_TRUE(IsImportanceBlock(Entity_Type_Block_Importance, 2));
		EXPECT_FALSE(IsImportanceBlock(static_cast<EntityType>(0), 2));
		EXPECT_FALSE(IsImportanceBlock(Entity_Type_Block_Normal, 2));
	}

	TEST(TEST_CLASS, GetBlockHeaderSize_ReturnsCorrectValueBasedOnEntityType) {
		constexpr uint32_t Importance_Block_Header_Size = sizeof(BlockHeader) + sizeof(ImportanceBlockFooter);
		constexpr uint32_t Padded_Block_Header_Size = sizeof(BlockHeader) + sizeof(PaddedBlockFooter);

		EXPECT_EQ(Padded_Block_Header_Size, GetBlockHeaderSize(Entity_Type_Block_Nemesis, 1));
		EXPECT_EQ(Padded_Block_Header_Size, GetBlockHeaderSize(Entity_Type_Block_Importance, 1));
		EXPECT_EQ(Padded_Block_Header_Size, GetBlockHeaderSize(static_cast<EntityType>(0), 1));
		EXPECT_EQ(Padded_Block_Header_Size, GetBlockHeaderSize(Entity_Type_Block_Normal, 1));

		EXPECT_EQ(Importance_Block_Header_Size, GetBlockHeaderSize(Entity_Type_Block_Nemesis, 2));
		EXPECT_EQ(Importance_Block_Header_Size, GetBlockHeaderSize(Entity_Type_Block_Importance, 2));
		EXPECT_EQ(Padded_Block_Header_Size, GetBlockHeaderSize(static_cast<EntityType>(0), 2));
		EXPECT_EQ(Padded_Block_Header_Size, GetBlockHeaderSize(Entity_Type_Block_Normal, 2));
	}

	TEST(TEST_CLASS, GetBlockHeaderDataBuffer_ReturnsCorrectValueBasedOnEntityType) {
		constexpr uint32_t Importance_Block_Header_Size = sizeof(BlockHeader)
				+ sizeof(ImportanceBlockFooter)
				- VerifiableEntity::Header_Size;
		constexpr uint32_t Padded_Block_Header_Size = sizeof(BlockHeader) - VerifiableEntity::Header_Size;

		AssertBlockHeaderDataBuffer(Padded_Block_Header_Size, Entity_Type_Block_Nemesis, 1);
		AssertBlockHeaderDataBuffer(Padded_Block_Header_Size, Entity_Type_Block_Importance, 1);
		AssertBlockHeaderDataBuffer(Padded_Block_Header_Size, static_cast<EntityType>(0), 1);
		AssertBlockHeaderDataBuffer(Padded_Block_Header_Size, Entity_Type_Block_Normal, 1);

		AssertBlockHeaderDataBuffer(Importance_Block_Header_Size, Entity_Type_Block_Nemesis, 2);
		AssertBlockHeaderDataBuffer(Importance_Block_Header_Size, Entity_Type_Block_Importance, 2);
		AssertBlockHeaderDataBuffer(Padded_Block_Header_Size, static_cast<EntityType>(0), 2);
		AssertBlockHeaderDataBuffer(Padded_Block_Header_Size, Entity_Type_Block_Normal, 2);
	}

	TEST(TEST_CLASS, GetTransactionPayloadSize_ReturnsCorrectValueBasedOnEntityType) {
		AssertTransactionPayloadSize(Entity_Type_Block_Nemesis, 1);
		AssertTransactionPayloadSize(Entity_Type_Block_Importance, 1);
		AssertTransactionPayloadSize(static_cast<EntityType>(0), 1);
		AssertTransactionPayloadSize(Entity_Type_Block_Normal, 1);

		AssertTransactionPayloadSize(Entity_Type_Block_Nemesis, 2);
		AssertTransactionPayloadSize(Entity_Type_Block_Importance, 2);
		AssertTransactionPayloadSize(static_cast<EntityType>(0), 2);
		AssertTransactionPayloadSize(Entity_Type_Block_Normal, 2);
	}

	// endregion

	// region block traits + test utils

	namespace {
		struct BlockNormalTraits {
			using FooterType = PaddedBlockFooter;
			static constexpr uint32_t Header_Size = sizeof(BlockHeader) + sizeof(FooterType);

			static std::unique_ptr<Block> CreateBlockWithTransactions(size_t numTransactions = 3) {
				auto transactions = test::GenerateRandomTransactions(numTransactions);
				return test::GenerateBlockWithTransactions(transactions);
			}
		};

		struct BlockImportanceTraits {
			using FooterType = ImportanceBlockFooter;
			static constexpr uint32_t Header_Size = sizeof(BlockHeader) + sizeof(FooterType);

			static std::unique_ptr<Block> CreateBlockWithTransactions(size_t numTransactions = 3) {
				auto transactions = test::GenerateRandomTransactions(numTransactions);
				return test::GenerateImportanceBlockWithTransactions(transactions);
			}
		};

		template<typename TTraits>
		std::unique_ptr<Block> CreateBlockWithReportedSize(uint32_t size) {
			auto pBlock = TTraits::CreateBlockWithTransactions();
			pBlock->Size = size;
			return pBlock;
		}

		mocks::MockTransaction& GetSecondTransaction(Block& block) {
			uint8_t* pBytes = reinterpret_cast<uint8_t*>(block.TransactionsPtr());
			auto firstTransactionSize = block.TransactionsPtr()->Size;
			auto paddingSize = utils::GetPaddingSize(firstTransactionSize, 8);
			return *reinterpret_cast<mocks::MockTransaction*>(pBytes + firstTransactionSize + paddingSize);
		}
	}

	// endregion

	// region transactions

	namespace {
		using ConstTraits = test::ConstTraitsT<Block>;
		using NonConstTraits = test::NonConstTraitsT<Block>;
	}

#define DATA_POINTER_TEST(TEST_NAME) \
	template<typename TAccessTraits, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Normal_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstTraits, BlockNormalTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Normal_NonConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonConstTraits, BlockNormalTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Importance_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstTraits, BlockImportanceTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Importance_NonConst) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonConstTraits, BlockImportanceTraits>(); \
	} \
	template<typename TAccessTraits, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	DATA_POINTER_TEST(TransactionsAreInaccessibleWhenBlockHasNoTransactions) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize<TTraits>(sizeof(BlockHeader));
		auto& accessor = TAccessTraits::GetAccessor(*pBlock);

		// Act + Assert:
		EXPECT_FALSE(!!accessor.TransactionsPtr());
		EXPECT_EQ(0u, test::CountContainerEntities(accessor.Transactions()));
	}

	DATA_POINTER_TEST(TransactionsAreAccessibleWhenBlockHasTransactions) {
		// Arrange:
		auto pBlock = TTraits::CreateBlockWithTransactions();
		const auto* pBlockEnd = test::AsVoidPointer(reinterpret_cast<uint8_t*>(pBlock.get()) + TTraits::Header_Size);
		auto& accessor = TAccessTraits::GetAccessor(*pBlock);

		// Act + Assert:
		EXPECT_EQ(pBlockEnd, accessor.TransactionsPtr());
		EXPECT_EQ(3u, test::CountContainerEntities(accessor.Transactions()));
	}

	// endregion

	// region IsSizeValid - no transactions

#define IS_SIZE_VALID_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Normal) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockNormalTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Importance) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockImportanceTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		bool IsSizeValid(const Block& block, mocks::PluginOptionFlags options = mocks::PluginOptionFlags::Default) {
			auto registry = mocks::CreateDefaultTransactionRegistry(options);
			return IsSizeValid(block, registry);
		}
	}

	IS_SIZE_VALID_TEST(SizeIsInvalidWhenReportedSizeIsZero) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize<TTraits>(0);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	IS_SIZE_VALID_TEST(SizeIsInvalidWhenReportedSizeIsLessThanHeaderSize) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize<TTraits>(sizeof(VerifiableEntity) - 1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	IS_SIZE_VALID_TEST(SizeIsInvalidWhenReportedSizeIsLessThanDerivedHeaderSize) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize<TTraits>(TTraits::Header_Size - 1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	IS_SIZE_VALID_TEST(SizeIsValidWhenReportedSizeIsEqualToHeaderSize) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize<TTraits>(TTraits::Header_Size);

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pBlock));
	}

	// endregion

	// region IsSizeValid - invalid inner tx sizes

	IS_SIZE_VALID_TEST(SizeIsInvalidWhenAnyTransactionHasPartialHeader) {
		// Arrange: create a block with 1 extra byte (which should be interpeted as a partial tx header)
		auto pBlock = CreateBlockWithReportedSize<TTraits>(TTraits::Header_Size + 1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	IS_SIZE_VALID_TEST(SizeIsInvalidWhenAnyTransactionHasInvalidSize) {
		// Arrange:
		auto pBlock = TTraits::CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Data.Size = 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	IS_SIZE_VALID_TEST(SizeIsInvalidWhenAnyTransactionHasZeroSize) {
		// Arrange:
		auto pBlock = TTraits::CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Size = 0;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	IS_SIZE_VALID_TEST(SizeIsInvalidWhenAnyInnerTransactionExpandsBeyondBuffer) {
		// Arrange:
		auto pBlock = TTraits::CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Size = pBlock->Size - pBlock->TransactionsPtr()->Size + 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	// endregion

	// region IsSizeValid - invalid inner tx types

	IS_SIZE_VALID_TEST(SizeIsInvalidWhenAnyTransactionHasUnknownType) {
		// Arrange:
		auto pBlock = TTraits::CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Type = static_cast<EntityType>(std::numeric_limits<uint16_t>::max());

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	IS_SIZE_VALID_TEST(SizeIsValidWhenAnyTransactionDoesNotSupportEmbedding) {
		// Arrange:
		auto pBlock = TTraits::CreateBlockWithTransactions();

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pBlock, mocks::PluginOptionFlags::Not_Embeddable));
	}

	// endregion

	// region IsSizeValid - valid transactions

	IS_SIZE_VALID_TEST(SizeIsInvalidWhenBlockWithTransactionsHasLargerReportedSizeThanActual) {
		// Arrange:
		auto pBlock = TTraits::CreateBlockWithTransactions();
		++pBlock->Size;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	IS_SIZE_VALID_TEST(SizeIsValidWhenReportedSizeIsEqualToHeaderSizePlusTransactionsSize) {
		// Arrange:
		auto pBlock = TTraits::CreateBlockWithTransactions();

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pBlock));
	}

	// endregion
}}
