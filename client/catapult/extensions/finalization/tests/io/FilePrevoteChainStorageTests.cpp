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

#include "finalization/src/io/FilePrevoteChainStorage.h"
#include "catapult/io/FileBlockStorage.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>
#include <fstream>

namespace fs = boost::filesystem;

namespace catapult { namespace io {

#define TEST_CLASS FilePrevoteChainStorageTests

	// region helpers

	namespace {
		using Blocks = std::vector<std::shared_ptr<const model::Block>>;

		constexpr model::FinalizationRound Default_Round{ FinalizationEpoch(123), FinalizationPoint(42) };

		Blocks ExtractBlocks(const BlockStorageView& blockStorageView, Height startHeight, Height endHeight) {
			Blocks blocks;
			for (auto height = startHeight; height <= endHeight; height = height + Height(1))
				blocks.push_back(blockStorageView.loadBlock(height));

			return blocks;
		}

		std::vector<uint8_t> ReadFile(const fs::path& filename) {
			std::vector<uint8_t> buffer;
			buffer.resize(fs::file_size(filename));

			std::ifstream fin(filename.generic_string(), std::ios_base::in | std::ios_base::binary);
			fin.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
			return buffer;
		}

		void AssertBlocks(const Blocks& expectedBlocks, const fs::path& directoryPath, Height height) {
			// Assert: load blocks and verify against expectedBlocks
			auto i = 0u;
			for (const auto& pExpectedBlock : expectedBlocks) {
				auto filename = directoryPath / (std::to_string(height.unwrap() + i) + ".dat");
				auto buffer = ReadFile(filename);
				const auto& block = reinterpret_cast<model::Block&>(buffer[0]);
				EXPECT_EQ(*pExpectedBlock, block) << i;
				++i;
			}
		}

		void AssertBlocks(const Blocks& savedBlocks, size_t numExpectedBlocks, const model::BlockRange& blockRange) {
			ASSERT_EQ(numExpectedBlocks, blockRange.size());

			auto i = 0u;
			for (const auto& block : blockRange) {
				EXPECT_EQ(*savedBlocks[i], block) << i;
				++i;
			}
		}

		template<typename T>
		auto Join(const T& t) {
			return fs::path(t);
		}

		template<typename First, typename... Args>
		auto Join(const First& first, const Args&... args) {
			return fs::path(first) / Join(args...);
		}
	}

	// endregion

	// region contains

	TEST(TEST_CLASS, ContainsReturnFalseWhenNoBackupIsPresent) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		FilePrevoteChainStorage storage(tempDir.name());

		// Act + Assert:
		EXPECT_FALSE(storage.contains(Default_Round, { Height(), Hash256() }));
	}

	namespace {
		void RunContainsTest(const consumer<const FilePrevoteChainStorage&, const Blocks&>& assertStorage) {
			// Arrange:
			test::TempDirectoryGuard tempDir;
			auto pStorageCache = mocks::CreateMemoryBlockStorageCache(12);
			auto blocks = ExtractBlocks(pStorageCache->view(), Height(10), Height(12));

			FilePrevoteChainStorage storage(tempDir.name());
			storage.save(pStorageCache->view(), { Default_Round, Height(10), 3 });

			// Act + Assert:
			assertStorage(storage, blocks);
		}

		auto BlockToHeightHashPair(const model::Block& block) {
			auto blockElement = test::BlockToBlockElement(block);
			return model::HeightHashPair{ block.Height, blockElement.EntityHash };
		}
	}

	TEST(TEST_CLASS, ContainsReturnFalseWhenRoundDoesNotExist) {
		// Arrange:
		RunContainsTest([](const auto& storage, const auto& blocks) {
			// Assert;
			auto round = Default_Round;
			round.Point = round.Point + FinalizationPoint(1);
			EXPECT_FALSE(storage.contains(round, BlockToHeightHashPair(*blocks[0])));
		});
	}

	TEST(TEST_CLASS, ContainsReturnFalseWhenHeightIsOutsideBackup) {
		// Arrange:
		RunContainsTest([](const auto& storage, const auto&) {
			// Assert;
			EXPECT_FALSE(storage.contains(Default_Round, { Height(13), Hash256() }));
		});
	}

	TEST(TEST_CLASS, ContainsReturnFalseWhenHashDoesNotMatch) {
		// Arrange:
		RunContainsTest([](const auto& storage, const auto&) {
			// Assert;
			EXPECT_FALSE(storage.contains(Default_Round, { Height(12), Hash256() }));
		});
	}

	TEST(TEST_CLASS, ContainsReturnTrueWhenHeightAndHashMatch) {
		// Arrange:
		RunContainsTest([](const auto& storage, const auto& blocks) {
			// Assert:
			for (const auto& pBlock : blocks)
				EXPECT_TRUE(storage.contains(Default_Round, BlockToHeightHashPair(*pBlock))) << pBlock->Height;
		});
	}

	// endregion

	// region load

	TEST(TEST_CLASS, CannotLoadUnsavedRound) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		FilePrevoteChainStorage storage(tempDir.name());

		// Act + Assert:
		EXPECT_THROW(storage.load(Default_Round, Height(100)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotLoadWhenIndexFileDoesNotExist) {
		// Arrange: prepare proper storage, but remove index file
		test::TempDirectoryGuard tempDir;
		auto pStorageCache = mocks::CreateMemoryBlockStorageCache(15);
		auto blocks = ExtractBlocks(pStorageCache->view(), Height(10), Height(15));

		FilePrevoteChainStorage storage(tempDir.name());
		storage.save(pStorageCache->view(), { Default_Round, Height(10), 0 });

		boost::filesystem::remove(Join(tempDir.name(), "voting", "123_42", "index.dat"));

		// Act + Assert:
		EXPECT_THROW(storage.load(Default_Round, Height(100)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanRoundtripZeroBlocks) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pStorageCache = mocks::CreateMemoryBlockStorageCache(15);
		auto blocks = ExtractBlocks(pStorageCache->view(), Height(10), Height(15));

		FilePrevoteChainStorage storage(tempDir.name());
		storage.save(pStorageCache->view(), { Default_Round, Height(10), 0 });

		// Act + Assert:
		auto blockRange = storage.load(Default_Round, Height(100));
		AssertBlocks(blocks, 0, blockRange);
	}

	TEST(TEST_CLASS, CanRoundtripSingleBlock) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pStorageCache = mocks::CreateMemoryBlockStorageCache(15);
		auto blocks = ExtractBlocks(pStorageCache->view(), Height(10), Height(15));

		FilePrevoteChainStorage storage(tempDir.name());
		storage.save(pStorageCache->view(), { Default_Round, Height(10), 1 });

		// Act:
		auto blockRange = storage.load(Default_Round, Height(100));
		AssertBlocks(blocks, 1, blockRange);
	}

	TEST(TEST_CLASS, CanRoundtripBlocks) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pStorageCache = mocks::CreateMemoryBlockStorageCache(15);
		auto blocks = ExtractBlocks(pStorageCache->view(), Height(10), Height(15));

		FilePrevoteChainStorage storage(tempDir.name());
		storage.save(pStorageCache->view(), { Default_Round, Height(10), 6 });

		// Act:
		auto blockRange = storage.load(Default_Round, Height(100));
		AssertBlocks(blocks, 6, blockRange);
	}

	TEST(TEST_CLASS, LoadRespectsHeightLimit) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pStorageCache = mocks::CreateMemoryBlockStorageCache(15);
		auto blocks = ExtractBlocks(pStorageCache->view(), Height(10), Height(15));

		FilePrevoteChainStorage storage(tempDir.name());
		storage.save(pStorageCache->view(), { Default_Round, Height(10), 6 });

		// Act:
		auto blockRange = storage.load(Default_Round, Height(13));
		AssertBlocks(blocks, 4, blockRange);
	}

	// endregion

	// region save

	TEST(TEST_CLASS, SaveCreatesCopyOfBlocks) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pStorageCache = mocks::CreateMemoryBlockStorageCache(15);
		auto blocks = ExtractBlocks(pStorageCache->view(), Height(10), Height(15));

		// Act:
		FilePrevoteChainStorage storage(tempDir.name());
		storage.save(pStorageCache->view(), { Default_Round, Height(12), 3 });

		// Assert:
		auto expectedPath = Join(tempDir.name(), "voting", "123_42");
		EXPECT_TRUE(fs::exists(expectedPath));

		Blocks expectedBlocks;
		std::move(blocks.begin() + 2, blocks.begin() + 2 + 3, std::back_inserter(expectedBlocks));

		AssertBlocks(expectedBlocks, expectedPath, Height(12));
	}

	TEST(TEST_CLASS, SaveIsRoundDependant) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pStorageCache = mocks::CreateMemoryBlockStorageCache(18);
		auto allBlocks = ExtractBlocks(pStorageCache->view(), Height(10), Height(18));

		// Act:
		FilePrevoteChainStorage storage1(tempDir.name());
		FilePrevoteChainStorage storage2(tempDir.name());
		FilePrevoteChainStorage storage3(tempDir.name());
		storage1.save(pStorageCache->view(), { { FinalizationEpoch(119), FinalizationPoint(59) }, Height(13), 3 });
		storage2.save(pStorageCache->view(), { { FinalizationEpoch(119), FinalizationPoint(60) }, Height(10), 3 });
		storage3.save(pStorageCache->view(), { { FinalizationEpoch(120), FinalizationPoint(59) }, Height(16), 3 });

		Blocks blocks1, blocks2, blocks3;
		std::move(allBlocks.begin() + 3, allBlocks.begin() + 6, std::back_inserter(blocks1));
		std::move(allBlocks.begin() + 0, allBlocks.begin() + 3, std::back_inserter(blocks2));
		std::move(allBlocks.begin() + 6, allBlocks.begin() + 9, std::back_inserter(blocks3));

		// Assert:
		AssertBlocks(blocks1, Join(tempDir.name(), "voting", "119_59"), Height(13));
		AssertBlocks(blocks2, Join(tempDir.name(), "voting", "119_60"), Height(10));
		AssertBlocks(blocks3, Join(tempDir.name(), "voting", "120_59"), Height(16));
	}

	TEST(TEST_CLASS, SavePurgesRoundDirectoryBeforeOverwriting) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pStorageCache = mocks::CreateMemoryBlockStorageCache(15);
		auto blocks = ExtractBlocks(pStorageCache->view(), Height(10), Height(15));

		FilePrevoteChainStorage storage(tempDir.name());
		storage.save(pStorageCache->view(), { Default_Round, Height(12), 3 });

		// Sanity: 3 blocks + index
		auto roundPath = Join(tempDir.name(), "voting", "123_42");
		EXPECT_EQ(4u, test::CountFilesAndDirectories(roundPath));
		EXPECT_TRUE(fs::exists(Join(roundPath, "14.dat")));
		EXPECT_FALSE(fs::exists(Join(roundPath, "15.dat")));

		// Act:
		storage.save(pStorageCache->view(), { Default_Round, Height(10), 3 });

		// Assert: 3 blocks + index
		EXPECT_EQ(4u, test::CountFilesAndDirectories(roundPath));
		EXPECT_TRUE(fs::exists(Join(roundPath, "12.dat")));
		EXPECT_FALSE(fs::exists(Join(roundPath, "13.dat")));
		EXPECT_FALSE(fs::exists(Join(roundPath, "14.dat")));

		Blocks expectedBlocks;
		std::move(blocks.begin(), blocks.begin() + 3, std::back_inserter(expectedBlocks));

		AssertBlocks(expectedBlocks, roundPath, Height(10));
	}

	TEST(TEST_CLASS, SaveCanOverwriteRound) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pStorageCache1 = mocks::CreateMemoryBlockStorageCache(12);
		auto blocks1 = ExtractBlocks(pStorageCache1->view(), Height(10), Height(12));

		FilePrevoteChainStorage storage(tempDir.name());
		storage.save(pStorageCache1->view(), { Default_Round, Height(10), 3 });

		// Sanity:
		auto roundPath = Join(tempDir.name(), "voting", "123_42");
		AssertBlocks(blocks1, roundPath, Height(10));

		// re-create 'new' blocks
		auto pStorageCache2 = mocks::CreateMemoryBlockStorageCache(12);
		auto blocks2 = ExtractBlocks(pStorageCache2->view(), Height(10), Height(12));

		// Act:
		storage.save(pStorageCache2->view(), { Default_Round, Height(10), 3 });

		// Assert:
		AssertBlocks(blocks2, roundPath, Height(10));
	}

	// endregion

	// region remove

	TEST(TEST_CLASS, RemoveIsNoOpWhenVotingDirectoryDoesNotExist) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto votingPath = Join(tempDir.name(), "voting");
		FilePrevoteChainStorage storage(tempDir.name());

		// Sanity:
		EXPECT_FALSE(fs::exists(votingPath));

		// Act + Assert:
		EXPECT_NO_THROW(storage.remove(Default_Round));
	}

	TEST(TEST_CLASS, RemoveIsNoOpWhenRoundDirectoryDoesNotExist) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto votingPath = Join(tempDir.name(), "voting");
		fs::create_directory(votingPath);
		FilePrevoteChainStorage storage(tempDir.name());

		// Sanity:
		EXPECT_TRUE(fs::exists(votingPath));
		EXPECT_FALSE(fs::exists(Join(votingPath, "123_42")));

		// Act + Assert:
		EXPECT_NO_THROW(storage.remove(Default_Round));
	}

	TEST(TEST_CLASS, CanRemoveRoundDirectory) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto pStorageCache = mocks::CreateMemoryBlockStorageCache(15);
		auto blocks = ExtractBlocks(pStorageCache->view(), Height(10), Height(15));

		FilePrevoteChainStorage storage(tempDir.name());
		storage.save(pStorageCache->view(), { Default_Round, Height(10), 6 });

		auto roundPath = Join(tempDir.name(), "voting", "123_42");

		// Sanity: 6 blocks + index
		EXPECT_TRUE(fs::exists(roundPath));
		EXPECT_EQ(7u, test::CountFilesAndDirectories(roundPath));

		// Act:
		storage.remove(Default_Round);

		// Assert:
		EXPECT_FALSE(fs::exists(roundPath));
		EXPECT_TRUE(fs::exists(Join(tempDir.name(), "voting")));
	}

	// endregion
}}
