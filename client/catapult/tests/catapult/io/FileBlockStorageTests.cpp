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

#include "catapult/io/FileBlockStorage.h"
#include "tests/test/core/BlockStorageTests.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"
#include <filesystem>

namespace catapult { namespace io {

#define TEST_CLASS FileBlockStorageTests

	namespace {
		struct FileTraits {
			using Guard = test::TempDirectoryGuard;
			using StorageType = FileBlockStorage;

			static std::unique_ptr<StorageType> OpenStorage(const std::string& destination, uint32_t fileDatabaseBatchSize = 1) {
				return std::make_unique<StorageType>(destination, fileDatabaseBatchSize);
			}

			static std::unique_ptr<StorageType> PrepareStorage(const std::string& destination, Height height = Height()) {
				test::PrepareStorage(destination);
				if (Height() != height)
					test::FakeHeight(destination, height.unwrap());

				return OpenStorage(destination, test::File_Database_Batch_Size);
			}
		};
	}

	DEFINE_BLOCK_STORAGE_TESTS(FileTraits)
	DEFINE_PRUNABLE_BLOCK_STORAGE_TESTS(FileTraits)

	// region modes

	TEST(TEST_CLASS, HashIndexCanBeEnabled) {
		// Arrange: prepare a directory with a hashes file
		test::TempDirectoryGuard tempDir;
		FileTraits::PrepareStorage(tempDir.name());

		// - purge the nemesis block
		FileBlockStorage storage(tempDir.name(), test::File_Database_Batch_Size, FileBlockStorageMode::Hash_Index);
		storage.dropBlocksAfter(Height());

		// - save a block
		auto pBlock = test::GenerateBlockWithTransactions(5, Height(1));
		auto blockElement = test::CreateBlockElementForSaveTests(*pBlock);
		storage.saveBlock(blockElement);

		// Act:
		auto pStorageBlockElement = storage.loadBlockElement(Height(1));
		auto hashes = storage.loadHashesFrom(Height(1), 100);

		// Assert: hashes are present
		ASSERT_EQ(1u, hashes.size());
		EXPECT_EQ(blockElement.EntityHash, *hashes.cbegin());
		test::AssertEqual(blockElement, *pStorageBlockElement);
	}

	TEST(TEST_CLASS, HashIndexCanBeDisabled) {
		// Arrange: prepare a directory without a hashes file
		test::TempDirectoryGuard tempDir;
		FileBlockStorage storage(tempDir.name(), test::File_Database_Batch_Size, FileBlockStorageMode::None);

		// - save a block
		auto pBlock = test::GenerateBlockWithTransactions(5, Height(1));
		auto blockElement = test::CreateBlockElementForSaveTests(*pBlock);
		storage.saveBlock(blockElement);

		// Act:
		auto pStorageBlockElement = storage.loadBlockElement(Height(1));

		// Assert: hashes are not present
		EXPECT_THROW(storage.loadHashesFrom(Height(1), 100), catapult_invalid_argument);
		test::AssertEqual(blockElement, *pStorageBlockElement);
	}

	// endregion

	// region folder management

	TEST(TEST_CLASS, PurgeDoesNotDeleteDataDirectory) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		FileBlockStorage storage(tempDir.name(), test::File_Database_Batch_Size);

		// Sanity:
		EXPECT_TRUE(std::filesystem::exists(tempDir.name()));

		// Act:
		storage.purge();

		// Assert:
		EXPECT_TRUE(std::filesystem::exists(tempDir.name()));
	}

	// endregion

	// region storage trailing data

	TEST(TEST_CLASS, CannotReadSavedBlockElementWithTrailingData) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pBlock = test::GenerateBlockWithTransactions(5, Height(2));
		auto element = test::BlockToBlockElement(*pBlock, test::GenerateRandomByteArray<Hash256>());
		{
			auto pStorage = FileTraits::PrepareStorage(tempDir.name());
			pStorage->saveBlock(element);
		}

		// - append some data
		{
			io::RawFile file(tempDir.name() + "/00000/00000.dat", io::OpenMode::Read_Append);
			file.seek(file.size());
			std::vector<uint8_t> buffer{ 42 };
			file.write(buffer);
		}

		// Act + Assert
		FileBlockStorage storage(tempDir.name(), test::File_Database_Batch_Size);
		EXPECT_THROW(storage.loadBlockElement(Height(2)), catapult_runtime_error);
	}

	// endregion

	// region disk persistence

	// these tests do not make sense for memory-based storage because blocks stored in memory-based storage
	// do not persist across instances

	TEST(TEST_CLASS, CanReadSavedBlockAcrossDifferentStorageInstances) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pBlock = test::GenerateBlockWithTransactions(5, Height(2));
		auto element = test::BlockToBlockElement(*pBlock, test::GenerateRandomByteArray<Hash256>());
		{
			auto pStorage = FileTraits::PrepareStorage(tempDir.name());
			pStorage->saveBlock(element);
		}

		// Act:
		FileBlockStorage storage(tempDir.name(), test::File_Database_Batch_Size);
		auto pBlockElement = storage.loadBlockElement(Height(2));

		// Assert:
		test::AssertEqual(element, *pBlockElement);
	}

	TEST(TEST_CLASS, CanReadMultipleSavedBlocksAcrossDifferentStorageInstances) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pBlock1 = test::GenerateBlockWithTransactions(5, Height(2));
		auto pBlock2 = test::GenerateBlockWithTransactions(5, Height(3));
		auto element1 = test::BlockToBlockElement(*pBlock1, test::GenerateRandomByteArray<Hash256>());
		auto element2 = test::BlockToBlockElement(*pBlock2, test::GenerateRandomByteArray<Hash256>());
		{
			auto pStorage = FileTraits::PrepareStorage(tempDir.name());
			pStorage->saveBlock(element1);
			pStorage->saveBlock(element2);
		}

		// Act:
		FileBlockStorage storage(tempDir.name(), test::File_Database_Batch_Size);
		auto pBlockElement1 = storage.loadBlockElement(Height(2));
		auto pBlockElement2 = storage.loadBlockElement(Height(3));

		// Assert:
		test::AssertEqual(element1, *pBlockElement1);
		test::AssertEqual(element2, *pBlockElement2);
	}

	// endregion
}}
