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

#include "catapult/io/FileBlockStorage.h"
#include "tests/test/core/BlockStorageTests.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace io {

#define TEST_CLASS FileBlockStorageTests

	namespace {
		struct FileTraits {
			using Guard = test::TempDirectoryGuard;
			using StorageType = FileBlockStorage;

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

	DEFINE_BLOCK_STORAGE_TESTS(FileTraits)

	// region disk persistence

	// these tests do not make sense for memory-based storage because blocks stored in memory-based storage
	// do not persist across instances

	TEST(TEST_CLASS, CanReadSavedBlockAcrossDifferentStorageInstances) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(Height(2));
		auto element = test::BlockToBlockElement(*pBlock, test::GenerateRandomData<Hash256_Size>());
		{
			auto pStorage = FileTraits::PrepareStorage(tempDir.name());
			pStorage->saveBlock(element);
		}

		// Act:
		FileBlockStorage storage(tempDir.name());
		auto pBlockElement = storage.loadBlockElement(Height(2));

		// Assert:
		test::AssertEqual(element, *pBlockElement);
	}

	TEST(TEST_CLASS, CanReadMultipleSavedBlocksAcrossDifferentStorageInstances) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pBlock1 = test::GenerateBlockWithTransactionsAtHeight(Height(2));
		auto pBlock2 = test::GenerateBlockWithTransactionsAtHeight(Height(3));
		auto element1 = test::BlockToBlockElement(*pBlock1, test::GenerateRandomData<Hash256_Size>());
		auto element2 = test::BlockToBlockElement(*pBlock2, test::GenerateRandomData<Hash256_Size>());
		{
			auto pStorage = FileTraits::PrepareStorage(tempDir.name());
			pStorage->saveBlock(element1);
			pStorage->saveBlock(element2);
		}

		// Act:
		FileBlockStorage storage(tempDir.name());
		auto pBlockElement1 = storage.loadBlockElement(Height(2));
		auto pBlockElement2 = storage.loadBlockElement(Height(3));

		// Assert:
		test::AssertEqual(element1, *pBlockElement1);
		test::AssertEqual(element2, *pBlockElement2);
	}

	// endregion
}}
