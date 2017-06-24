#include "catapult/model/Block.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionContainerTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"

namespace catapult { namespace model {

#define TEST_CLASS BlockTests

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(VerifiableEntity) // base
			+ sizeof(uint64_t) // height
			+ sizeof(uint64_t) // timestamp
			+ sizeof(uint64_t) // difficulty
			+ Hash256_Size // previous block hash
			+ Hash256_Size; // block transactions hash

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(Block));
		EXPECT_EQ(104u + 88u, sizeof(Block));
	}

	// region test utils

	namespace {
		std::unique_ptr<Block> CreateBlockWithTransactions(size_t numTransactions = 3) {
			auto transactions = test::GenerateRandomTransactions(numTransactions);
			return test::GenerateRandomBlockWithTransactions(transactions);
		}

		std::unique_ptr<Block> CreateBlockWithReportedSize(uint32_t size) {
			auto pBlock = CreateBlockWithTransactions();
			pBlock->Size = size;
			return pBlock;
		}

		mocks::MockTransaction& GetSecondTransaction(Block& block) {
			uint8_t* pBytes = reinterpret_cast<uint8_t*>(block.TransactionsPtr());
			return *reinterpret_cast<mocks::MockTransaction*>(pBytes + block.TransactionsPtr()->Size);
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
		auto pBlock = CreateBlockWithReportedSize(sizeof(Block));
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

	// region IsSizeValid

	namespace {
		bool IsSizeValid(const Block& block, mocks::PluginOptionFlags options = mocks::PluginOptionFlags::Default) {
			auto pRegistry = mocks::CreateDefaultTransactionRegistry(options);
			return IsSizeValid(block, *pRegistry);
		}
	}

	// region no transactions

	TEST(TEST_CLASS, SizeInvalidIfReportedSizeIsZero) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize(0);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeInvalidIfReportedSizeIsLessThanHeaderSize) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize(sizeof(Block) - 1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeValidIfReportedSizeIsEqualToHeaderSize) {
		// Arrange:
		auto pBlock = CreateBlockWithReportedSize(sizeof(Block));

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pBlock));
	}

	// endregion

	// region invalid inner tx sizes

	TEST(TEST_CLASS, SizeInvalidIfAnyTransactionHasPartialHeader) {
		// Arrange: create a block with 1 extra byte (which should be interpeted as a partial tx header)
		auto pBlock = CreateBlockWithReportedSize(sizeof(Block) + 1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeInvalidIfAnyTransactionHasInvalidSize) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Data.Size = 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeInvalidIfAnyTransactionHasZeroSize) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Size = 0;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeInvalidIfAnyInnerTransactionExpandsBeyondBuffer) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Size = pBlock->Size - pBlock->TransactionsPtr()->Size + 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	// endregion

	// region invalid inner tx types

	TEST(TEST_CLASS, SizeInvalidIfAnyTransactionHasUnknownType) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		GetSecondTransaction(*pBlock).Type = static_cast<EntityType>(-1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeValidIfAnyTransactionDoesNotSupportEmbedding) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pBlock, mocks::PluginOptionFlags::Not_Embeddable));
	}

	// endregion

	TEST(TEST_CLASS, SizeInvalidIfBlockWithTransactionsHasLargerReportedSizeThanActual) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();
		++pBlock->Size;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pBlock));
	}

	TEST(TEST_CLASS, SizeValidIfReportedSizeIsEqualToHeaderSizePlusTransactionsSize) {
		// Arrange:
		auto pBlock = CreateBlockWithTransactions();

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pBlock));
	}

	// endregion
}}
