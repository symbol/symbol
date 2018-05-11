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

#include "catapult/io/FileBasedStorage.h"
#include "tests/catapult/io/test/BlockStorageTestUtils.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

using catapult::test::TempDirectoryGuard;

namespace catapult { namespace io {

#define TEST_CLASS FileBasedStorageTests

	namespace {
		struct FileBasedTraits {
			using Guard = TempDirectoryGuard;
			using StorageType = FileBasedStorage;

			static std::unique_ptr<StorageType> OpenStorage(const std::string& destination) {
				return std::make_unique<StorageType>(destination);
			}

			static std::unique_ptr<StorageType> PrepareStorage(const std::string& destination, Height height = Height()) {
				test::PrepareStorage(destination);
				if (Height() != height)
					test::FakeHeight(destination, height.unwrap());
				return OpenStorage(destination);
			}
		};
	}

	DEFINE_BLOCK_STORAGE_TESTS(FileBasedTraits)

	// these tests do not make sense for memory-based storage because blocks stored in memory-based storage
	// do not persist across instances

	TEST(TEST_CLASS, CanReadSavedBlockAcrossDifferentStorageInstances) {
		// Arrange:
		TempDirectoryGuard tempDir;
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(Height(2));
		auto element = test::BlockToBlockElement(*pBlock, test::GenerateRandomData<Hash256_Size>());
		{
			auto pStorage = FileBasedTraits::PrepareStorage(tempDir.name());
			pStorage->saveBlock(element);
		}

		// Act:
		FileBasedStorage storage(tempDir.name());
		auto pBlockElement = storage.loadBlockElement(Height(2));

		// Assert:
		test::AssertEqual(element, *pBlockElement);
	}

	TEST(TEST_CLASS, CanReadMultipleSavedBlocksAcrossDifferentStorageInstances) {
		// Arrange:
		TempDirectoryGuard tempDir;
		auto pBlock1 = test::GenerateBlockWithTransactionsAtHeight(Height(2));
		auto pBlock2 = test::GenerateBlockWithTransactionsAtHeight(Height(3));
		auto element1 = test::BlockToBlockElement(*pBlock1, test::GenerateRandomData<Hash256_Size>());
		auto element2 = test::BlockToBlockElement(*pBlock2, test::GenerateRandomData<Hash256_Size>());
		{
			auto pStorage = FileBasedTraits::PrepareStorage(tempDir.name());
			pStorage->saveBlock(element1);
			pStorage->saveBlock(element2);
		}

		// Act:
		FileBasedStorage storage(tempDir.name());
		auto pBlockElement1 = storage.loadBlockElement(Height(2));
		auto pBlockElement2 = storage.loadBlockElement(Height(3));

		// Assert:
		test::AssertEqual(element1, *pBlockElement1);
		test::AssertEqual(element2, *pBlockElement2);
	}

	namespace {
		// note: for test purposes, hardcoded 00000 dir is ok
		auto GetPath(const std::string& baseDirectory, uint64_t height) {
			std::stringstream pathBuilder;
			pathBuilder
					<< baseDirectory
					<< "/00000/"
					<< std::setw(5) << std::setfill('0') << height
					<< ".dat";

			return pathBuilder.str();
		}

		bool BlockFileExists(const std::string& baseDirectory, uint64_t height) {
			return boost::filesystem::exists(GetPath(baseDirectory, height));
		}

		void DeleteBlockFile(const std::string& baseDirectory, uint64_t height) {
			boost::filesystem::remove(GetPath(baseDirectory, height));
		}

		bool Contains(const std::set<uint64_t>& heights, uint64_t height) {
			return heights.end() != heights.find(height);
		}

		void AssertBlockFiles(const std::string& baseDirectory, const std::set<uint64_t>& heights) {
			auto maxHeight = *heights.rbegin();
			for (auto height = 1u; height <= maxHeight; ++height)
				if (Contains(heights, height))
					EXPECT_TRUE(BlockFileExists(baseDirectory, height)) << "block at height " << height;
				else
					EXPECT_FALSE(BlockFileExists(baseDirectory, height)) << "block at height " << height;
		}
	}

	TEST(TEST_CLASS, PruneBlocksBefore_CanPruneAtHeightBeforeChainHeight) {
		// Arrange:
		auto pStorage = test::PrepareStorageWithBlocks<FileBasedTraits>(10);

		// Act: prune will remove 2-7
		pStorage->pruneBlocksBefore(Height(8));

		// Assert:
		EXPECT_EQ(Height(10), pStorage->chainHeight());
		AssertBlockFiles(pStorage.pTempDirectoryGuard->name(), { 1, 8, 9, 10 });
	}

	TEST(TEST_CLASS, PruneBlocksBefore_CanPruneAtHeightEqualToChainHeight) {
		// Arrange:
		auto pStorage = test::PrepareStorageWithBlocks<FileBasedTraits>(10);

		// Act: prune will remove 2-9
		pStorage->pruneBlocksBefore(Height(10));

		// Assert:
		EXPECT_EQ(Height(10), pStorage->chainHeight());
		AssertBlockFiles(pStorage.pTempDirectoryGuard->name(), { 1, 10 });
	}

	TEST(TEST_CLASS, PruneBlocksBefore_ThrowsAtHeightAfterChainHeight) {
		// Arrange:
		auto pStorage = test::PrepareStorageWithBlocks<FileBasedTraits>(5);

		// Act + Assert:
		EXPECT_THROW(pStorage->pruneBlocksBefore(Height(10)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, PruneBlocksStopsOnFirstNonexistentFile) {
		// Arrange:
		auto pStorage = test::PrepareStorageWithBlocks<FileBasedTraits>(10);
		DeleteBlockFile(pStorage.pTempDirectoryGuard->name(), 4);

		// Act: prune will remove 5, 6, 7
		pStorage->pruneBlocksBefore(Height(8));

		// Assert:
		EXPECT_EQ(Height(10), pStorage->chainHeight());
		AssertBlockFiles(pStorage.pTempDirectoryGuard->name(), { 1, 2, 3, 8, 9, 10 });
	}
}}
