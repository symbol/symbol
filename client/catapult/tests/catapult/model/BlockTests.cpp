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

#include "catapult/model/Block.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionContainerTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/Alignment.h"

namespace catapult { namespace model {

#define TEST_CLASS BlockTests

	// region size + alignment

#define BLOCK_FIELDS \
	FIELD(Height) \
	FIELD(Timestamp) \
	FIELD(Difficulty) \
	FIELD(GenerationHashProof.Gamma) \
	FIELD(GenerationHashProof.VerificationHash) \
	FIELD(GenerationHashProof.Scalar) \
	FIELD(PreviousBlockHash) \
	FIELD(TransactionsHash) \
	FIELD(ReceiptsHash) \
	FIELD(StateHash) \
	FIELD(BeneficiaryAddress) \
	FIELD(FeeMultiplier)

	TEST(TEST_CLASS, BlockHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(VerifiableEntity) + Block::Footer_Size;

#define FIELD(X) expectedSize += sizeof(Block::X);
		BLOCK_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(BlockHeader));
		EXPECT_EQ(112u + 4 + 260, sizeof(BlockHeader));

		using BlockAlias = Block; // use alias to bypass lint rule
		EXPECT_EQ(sizeof(BlockHeader), sizeof(BlockAlias));
	}

	TEST(TEST_CLASS, BlockHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(Block, X);
		BLOCK_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(BlockHeader) % 8);
	}

#undef BLOCK_FIELDS

	// endregion

	// region test utils

	namespace {
		std::unique_ptr<Block> CreateBlockWithTransactions(size_t numTransactions = 3) {
			auto transactions = test::GenerateRandomTransactions(numTransactions);
			return test::GenerateBlockWithTransactions(transactions);
		}

		std::unique_ptr<Block> CreateBlockWithReportedSize(uint32_t size) {
			auto pBlock = CreateBlockWithTransactions();
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
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonConstTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	DATA_POINTER_TEST(TransactionsAreInaccessibleWhenBlockHasNoTransactions) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize(sizeof(BlockHeader));
		auto& accessor = TTraits::GetAccessor(*pBlock);

		// Act + Assert:
		EXPECT_FALSE(!!accessor.TransactionsPtr());
		EXPECT_EQ(0u, test::CountTransactions(accessor.Transactions()));
	}

	DATA_POINTER_TEST(TransactionsAreAccessibleWhenBlockHasTransactions) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		const auto* pBlockEnd = test::AsVoidPointer(pBlock.get() + 1);
		auto& accessor = TTraits::GetAccessor(*pBlock);

		// Act + Assert:
		EXPECT_EQ(pBlockEnd, accessor.TransactionsPtr());
		EXPECT_EQ(3u, test::CountTransactions(accessor.Transactions()));
	}

	// endregion

	// region GetTransactionPayloadSize

	TEST(TEST_CLASS, GetTransactionPayloadSizeReturnsCorrectPayloadSize) {
		// Arrange:
		BlockHeader header;
		header.Size = sizeof(BlockHeader) + 123;

		// Act:
		auto payloadSize = GetTransactionPayloadSize(header);

		// Assert:
		EXPECT_EQ(123u, payloadSize);
	}

	// endregion

	namespace {
		bool IsSizeValid(const Block& block, mocks::PluginOptionFlags options = mocks::PluginOptionFlags::Default) {
			auto registry = mocks::CreateDefaultTransactionRegistry(options);
			return IsSizeValid(block, registry);
		}
	}

	// region IsSizeValid - no transactions

	TEST(TEST_CLASS, SizeInvalidWhenReportedSizeIsZero) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize(0);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeInvalidWhenReportedSizeIsLessThanHeaderSize) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize(sizeof(BlockHeader) - 1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeValidWhenReportedSizeIsEqualToHeaderSize) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize(sizeof(BlockHeader));

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pBlock));
	}

	// endregion

	// region IsSizeValid - invalid inner tx sizes

	TEST(TEST_CLASS, SizeInvalidWhenAnyTransactionHasPartialHeader) {
		// Arrange: create a block with 1 extra byte (which should be interpeted as a partial tx header)
		auto pBlock = CreateBlockWithReportedSize(sizeof(BlockHeader) + 1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeInvalidWhenAnyTransactionHasInvalidSize) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Data.Size = 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeInvalidWhenAnyTransactionHasZeroSize) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Size = 0;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeInvalidWhenAnyInnerTransactionExpandsBeyondBuffer) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Size = pBlock->Size - pBlock->TransactionsPtr()->Size + 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	// endregion

	// region IsSizeValid - invalid inner tx types

	TEST(TEST_CLASS, SizeInvalidWhenAnyTransactionHasUnknownType) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Type = static_cast<EntityType>(-1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeValidWhenAnyTransactionDoesNotSupportEmbedding) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pBlock, mocks::PluginOptionFlags::Not_Embeddable));
	}

	// endregion

	// region IsSizeValid - valid transactions

	TEST(TEST_CLASS, SizeInvalidWhenBlockWithTransactionsHasLargerReportedSizeThanActual) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		++pBlock->Size;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeValidWhenReportedSizeIsEqualToHeaderSizePlusTransactionsSize) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pBlock));
	}

	// endregion
}}
