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

#include "catapult/io/MoveBlockFiles.h"
#include "tests/test/core/BlockStatementTestUtils.h"
#include "tests/test/core/BlockStorageTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS MoveBlockFilesTests

	namespace {
		// region block element with block

		struct BlockElementWithBlock {
		public:
			BlockElementWithBlock(std::unique_ptr<model::Block>&& pSourceBlock, model::BlockElement&& blockElement)
					: pBlock(std::move(pSourceBlock))
					, BlockElement(std::move(blockElement))
			{}

		public:
			std::unique_ptr<model::Block> pBlock;
			model::BlockElement BlockElement;
		};

		using BlockElements = std::vector<BlockElementWithBlock>;

		void AppendBlockElement(BlockElements& blockElements, Height height) {
			auto pBlock = test::GenerateBlockWithTransactions(5, Height(height));
			auto blockElement = test::CreateBlockElementForSaveTests(*pBlock);
			blockElements.emplace_back(std::move(pBlock), std::move(blockElement));
		}

		template<typename TTraits>
		auto CreateBlockElements(uint64_t startHeight, uint64_t endHeight) {
			BlockElements blockElements;
			for (auto height = startHeight; height <= endHeight; ++height)
				TTraits::AppendBlockElement(blockElements, Height(height));

			return blockElements;
		}

		// endregion

		void PopulateBlockStorage(BlockStorage& storage, const BlockElements& blockElements) {
			// use dropBlocksAfter, so that mock storage thinks current height is larger than it actually is
			auto startHeight = blockElements[0].pBlock->Height;
			if (startHeight > Height(0))
				storage.dropBlocksAfter(startHeight - Height(1));

			for (const auto& blockElementPair : blockElements)
				storage.saveBlock(blockElementPair.BlockElement);
		}

		void AssertStorage(const BlockElements& blockElements, const BlockStorage& storage) {
			for (const auto& blockElementPair : blockElements) {
				auto pBlockElement = test::LoadBlockElementWithStatements(storage, blockElementPair.pBlock->Height);
				test::AssertEqual(blockElementPair.BlockElement, *pBlockElement);
			}
		}

		// region traits

		struct BlocksWithoutStatementTraits {
			static constexpr auto AppendBlockElement = io::AppendBlockElement;
		};

		struct BlocksWithStatementTraits {
			static void AppendBlockElement(BlockElements& blockElements, Height height) {
				io::AppendBlockElement(blockElements, height);
				blockElements.back().BlockElement.OptionalStatement = test::GenerateRandomStatements({ 3, 5, 7 });
			}
		};

		// endregion
	}

	// region MoveBlockFiles

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_WithoutStatements) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlocksWithoutStatementTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_WithStatements) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlocksWithStatementTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(CanMoveBlockFilesWhenDestinationHasNormalChain) {
		// Arrange: destination 0 blocks, source 4 blocks
		auto destination = mocks::MockMemoryBlockStorage();
		auto source = mocks::MockMemoryBlockStorage();
		auto sourceBlocks = CreateBlockElements<TTraits>(2, 5);

		PopulateBlockStorage(source, sourceBlocks);

		// Act:
		MoveBlockFiles(source, destination, Height(2));

		// Assert: blocks are present in destination, source storage is empty
		AssertStorage(sourceBlocks, destination);
		EXPECT_EQ(Height(0), source.chainHeight());
	}

	TRAITS_BASED_TEST(CanMoveBlockFilesWhenDestinationHasForkedChain_SingleBlock) {
		// Arrange: destination 4 blocks, source 1 block
		auto destination = mocks::MockMemoryBlockStorage();
		auto source = mocks::MockMemoryBlockStorage();
		auto destinationBlocks = CreateBlockElements<TTraits>(2, 5);
		auto sourceBlocks = CreateBlockElements<TTraits>(5, 5);

		PopulateBlockStorage(destination, destinationBlocks);
		PopulateBlockStorage(source, sourceBlocks);

		// Act: move only single block
		MoveBlockFiles(source, destination, Height(5));

		// Assert: blocks are present in destination, source storage is empty
		AssertStorage(sourceBlocks, destination);
		EXPECT_EQ(Height(0), source.chainHeight());
	}

	TRAITS_BASED_TEST(CanMoveBlockFilesWhenDestinationHasForkedChain_MultipleBlocks) {
		// Arrange: destination 4 blocks, source 2 blocks
		auto destination = mocks::MockMemoryBlockStorage();
		auto source = mocks::MockMemoryBlockStorage();
		auto destinationBlocks = CreateBlockElements<TTraits>(2, 5);
		auto sourceBlocks = CreateBlockElements<TTraits>(3, 4);

		PopulateBlockStorage(destination, destinationBlocks);
		PopulateBlockStorage(source, sourceBlocks);

		// Act:
		MoveBlockFiles(source, destination, Height(3));

		// Assert: blocks are present in destination, source storage is empty
		AssertStorage(sourceBlocks, destination);
		EXPECT_EQ(Height(0), source.chainHeight());
	}

	TRAITS_BASED_TEST(CanMoveBlockFilesWhenStartHeightIsOne) {
		// Arrange: destination 4 blocks, source 4 blocks
		auto destination = mocks::MockMemoryBlockStorage();
		auto source = mocks::MockMemoryBlockStorage();
		auto destinationBlocks = CreateBlockElements<TTraits>(2, 5);
		auto sourceBlocks = CreateBlockElements<TTraits>(1, 4);

		PopulateBlockStorage(destination, destinationBlocks);
		PopulateBlockStorage(source, sourceBlocks);

		// Act:
		MoveBlockFiles(source, destination, Height(1));

		// Assert: blocks are present in destination, source storage is empty
		AssertStorage(sourceBlocks, destination);
		EXPECT_EQ(Height(0), source.chainHeight());
	}

	TRAITS_BASED_TEST(MoveBlockFilesThrowsWhenStartHeightIsLessThanOne) {
		// Arrange: destination 0 blocks, source 4 blocks
		auto destination = mocks::MockMemoryBlockStorage();
		auto source = mocks::MockMemoryBlockStorage();
		auto sourceBlocks = CreateBlockElements<TTraits>(2, 5);

		PopulateBlockStorage(source, sourceBlocks);

		// Act + Assert:
		EXPECT_THROW(MoveBlockFiles(source, destination, Height(0)), catapult_invalid_argument);
	}

	// endregion
}}
