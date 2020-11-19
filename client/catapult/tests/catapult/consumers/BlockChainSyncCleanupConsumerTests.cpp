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

#include "catapult/consumers/BlockConsumers.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/FileQueue.h"
#include "tests/catapult/consumers/test/ConsumerTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <filesystem>

namespace catapult { namespace consumers {

#define TEST_CLASS BlockChainSyncCleanupConsumerTests

	namespace {
		static constexpr auto Index_Writer_Filename = "index_server.dat";
		static constexpr auto Index_Reader_Filename = "index_server_r.dat";

		uint64_t ReadIndexFileValue(const std::filesystem::path& indexFilePath) {
			return io::IndexFile(indexFilePath.generic_string()).get();
		}

		void ProduceThreeStateChangeMessages(const std::filesystem::path& stateChangeDirectory) {
			// Arrange: write three files
			io::FileQueueWriter writer(stateChangeDirectory.generic_string(), Index_Writer_Filename);
			for (auto i = 0u; i < 3; ++i) {
				writer.write(test::GenerateRandomVector(12 + i));
				writer.flush();
			}

			// Sanity:
			EXPECT_TRUE(std::filesystem::exists(stateChangeDirectory / "0000000000000000.dat"));
			EXPECT_TRUE(std::filesystem::exists(stateChangeDirectory / "0000000000000001.dat"));
			EXPECT_TRUE(std::filesystem::exists(stateChangeDirectory / "0000000000000002.dat"));

			EXPECT_EQ(3u, ReadIndexFileValue(stateChangeDirectory / Index_Writer_Filename));
			EXPECT_FALSE(std::filesystem::exists(stateChangeDirectory / Index_Reader_Filename));
		}
	}

	TEST(TEST_CLASS, ConsumerHasNoEffectWhenStateChangeDirectoryDoesNotExist) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto dataDirectory = config::CatapultDataDirectoryPreparer::Prepare(tempDir.name());

		// Act:
		auto result = CreateBlockChainSyncCleanupConsumer(tempDir.name())(disruptor::ConsumerInput());

		// Assert:
		test::AssertContinued(result);
	}

	TEST(TEST_CLASS, ConsumerDeletesOldestStateChangeMessages) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto dataDirectory = config::CatapultDataDirectoryPreparer::Prepare(tempDir.name());
		auto stateChangeDirectory = dataDirectory.spoolDir("state_change").path();
		ProduceThreeStateChangeMessages(stateChangeDirectory);

		// Act:
		auto result = CreateBlockChainSyncCleanupConsumer(tempDir.name())(disruptor::ConsumerInput());

		// Assert:
		test::AssertContinued(result);

		EXPECT_FALSE(std::filesystem::exists(stateChangeDirectory / "0000000000000000.dat"));
		EXPECT_FALSE(std::filesystem::exists(stateChangeDirectory / "0000000000000001.dat"));
		EXPECT_TRUE(std::filesystem::exists(stateChangeDirectory / "0000000000000002.dat"));

		EXPECT_EQ(3u, ReadIndexFileValue(stateChangeDirectory / Index_Writer_Filename));
		EXPECT_EQ(2u, ReadIndexFileValue(stateChangeDirectory / Index_Reader_Filename));
	}

	TEST(TEST_CLASS, ConsumerHasNoEffectWhenAllStateChangeMessagesHavePreviouslyBeenConsumed) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto dataDirectory = config::CatapultDataDirectoryPreparer::Prepare(tempDir.name());
		auto stateChangeDirectory = dataDirectory.spoolDir("state_change").path();
		ProduceThreeStateChangeMessages(stateChangeDirectory);

		io::FileQueueReader reader(stateChangeDirectory.generic_string(), Index_Reader_Filename, Index_Writer_Filename);
		for (auto i = 0u; i < 3; ++i)
			reader.tryReadNextMessage([](const auto&) {});

		// Sanity:
		EXPECT_EQ(3u, ReadIndexFileValue(stateChangeDirectory / Index_Writer_Filename));
		EXPECT_EQ(3u, ReadIndexFileValue(stateChangeDirectory / Index_Reader_Filename));

		// Act:
		auto result = CreateBlockChainSyncCleanupConsumer(tempDir.name())(disruptor::ConsumerInput());

		// Assert:
		test::AssertContinued(result);

		EXPECT_FALSE(std::filesystem::exists(stateChangeDirectory / "0000000000000000.dat"));
		EXPECT_FALSE(std::filesystem::exists(stateChangeDirectory / "0000000000000001.dat"));
		EXPECT_FALSE(std::filesystem::exists(stateChangeDirectory / "0000000000000002.dat"));

		EXPECT_EQ(3u, ReadIndexFileValue(stateChangeDirectory / Index_Writer_Filename));
		EXPECT_EQ(3u, ReadIndexFileValue(stateChangeDirectory / Index_Reader_Filename));
	}
}}
