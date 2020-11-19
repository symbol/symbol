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

#include "catapult/local/recovery/StorageStart.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS StorageStartTests

	namespace {
		auto CreateStorageWithBlocks(Height startHeight, size_t numBlocks) {
			auto pStorage = mocks::CreateMemoryBlockStorage(0);
			pStorage->dropBlocksAfter(startHeight - Height(1));
			for (auto i = 0u; i < numBlocks; ++i) {
				model::Block block;
				block.Size = sizeof(model::BlockHeader);
				block.Height = startHeight + Height(i);
				pStorage->saveBlock(test::BlockToBlockElement(block));
			}

			return pStorage;
		}
	}

	TEST(TEST_CLASS, FindStartHeightReturnsOneWhenChainIsComplete) {
		// Arrange:
		auto pStorage = mocks::CreateMemoryBlockStorage(10);

		// Act:
		auto startHeight = FindStartHeight(*pStorage);

		// Assert:
		EXPECT_EQ(Height(1), startHeight);
	}

	TEST(TEST_CLASS, FindStartHeightReturnsProperHeightWhenChainContainsSingleBlockAtChainHeight) {
		// Arrange:
		auto pStorage = CreateStorageWithBlocks(Height(22), 1);

		// Act:
		auto startHeight = FindStartHeight(*pStorage);

		// Assert:
		EXPECT_EQ(Height(22), startHeight);
	}

	TEST(TEST_CLASS, FindStartHeightReturnsProperHeightWhenChainContainsMultipleBlocksAtChainHeight) {
		// Arrange:
		auto pStorage = CreateStorageWithBlocks(Height(22), 5);

		// Act:
		auto startHeight = FindStartHeight(*pStorage);

		// Assert:
		EXPECT_EQ(Height(22), startHeight);
	}

	TEST(TEST_CLASS, FindStartHeightThrowsWhenChainDoesNotContainBlockAtChainHeight) {
		// Arrange: fake starting height, but do not save any blocks
		auto pStorage = CreateStorageWithBlocks(Height(22), 0);

		// Act + Assert:
		EXPECT_THROW(FindStartHeight(*pStorage), catapult_file_io_error);
	}
}}
