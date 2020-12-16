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

#include "catapult/io/FilesystemUtils.h"
#include "catapult/io/RawFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <filesystem>

namespace catapult { namespace io {

#define TEST_CLASS FilesystemUtilsTests

	// region PurgeDirectory

	TEST(TEST_CLASS, PurgeDirectory_HasNoEffectOnNonexistentDirectory) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto directoryPath = std::filesystem::path(tempDir.name()) / "staging";

		// Sanity:
		EXPECT_FALSE(std::filesystem::exists(directoryPath));

		// Act:
		PurgeDirectory(directoryPath.generic_string());

		// Assert:
		EXPECT_FALSE(std::filesystem::exists(directoryPath));
	}

	TEST(TEST_CLASS, PurgeDirectory_HasNoEffectOnEmptyDirectory) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto directoryPath = std::filesystem::path(tempDir.name()) / "staging";
		std::filesystem::create_directory(directoryPath);

		// Sanity:
		EXPECT_TRUE(std::filesystem::exists(directoryPath));

		// Act:
		PurgeDirectory(directoryPath.generic_string());

		// Assert:
		EXPECT_TRUE(std::filesystem::exists(directoryPath));
		EXPECT_EQ(0u, test::CountFilesAndDirectories(directoryPath));
	}

	TEST(TEST_CLASS, PurgeDirectory_RemovesAllSubFiles) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto directoryPath = std::filesystem::path(tempDir.name()) / "staging";
		std::filesystem::create_directory(directoryPath);

		// - create three files
		for (const auto& name : { "alpha", "beta", "gamma" })
			io::RawFile((directoryPath / name).generic_string(), io::OpenMode::Read_Write);

		// Sanity:
		EXPECT_EQ(3u, test::CountFilesAndDirectories(directoryPath));

		// Act:
		PurgeDirectory(directoryPath.generic_string());

		// Assert:
		EXPECT_TRUE(std::filesystem::exists(directoryPath));
		EXPECT_EQ(0u, test::CountFilesAndDirectories(directoryPath));
	}

	TEST(TEST_CLASS, PurgeDirectory_RemovesAllSubFolders) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto directoryPath = std::filesystem::path(tempDir.name()) / "staging";
		std::filesystem::create_directory(directoryPath);

		// - create three files inside of a subdirectory
		std::filesystem::create_directory(directoryPath / "sub");
		for (const auto& name : { "aaa", "bbb", "ccc" })
			io::RawFile((directoryPath / "sub" / name).generic_string(), io::OpenMode::Read_Write);

		// Sanity:
		EXPECT_EQ(1u, test::CountFilesAndDirectories(directoryPath));
		EXPECT_EQ(3u, test::CountFilesAndDirectories(directoryPath / "sub"));

		// Act:
		PurgeDirectory(directoryPath.generic_string());

		// Assert:
		EXPECT_TRUE(std::filesystem::exists(directoryPath));
		EXPECT_EQ(0u, test::CountFilesAndDirectories(directoryPath));

		EXPECT_FALSE(std::filesystem::exists(directoryPath / "sub"));
	}

	// endregion

	// region MoveAllFiles

	TEST(TEST_CLASS, MoveAllFiles_HasNoEffectWhenDirectoryIsEmpty) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto sourceDirectory = (std::filesystem::path(tempDir.name()) / "source").generic_string();
		auto destDirectory = (std::filesystem::path(tempDir.name()) / "dest").generic_string();

		std::filesystem::create_directory(sourceDirectory);
		std::filesystem::create_directory(destDirectory);

		// Act:
		MoveAllFiles(sourceDirectory, destDirectory);

		// Assert:
		EXPECT_EQ(0u, test::CountFilesAndDirectories(sourceDirectory));
		EXPECT_EQ(0u, test::CountFilesAndDirectories(destDirectory));
	}

	TEST(TEST_CLASS, MoveAllFiles_DoesNotMoveSubDirectories) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto sourceDirectory = (std::filesystem::path(tempDir.name()) / "source").generic_string();
		auto destDirectory = (std::filesystem::path(tempDir.name()) / "dest").generic_string();

		std::filesystem::create_directory(sourceDirectory);
		std::filesystem::create_directory(destDirectory);

		// - create three directories
		for (const auto& name : { "alpha", "beta", "gamma" })
			std::filesystem::create_directory(std::filesystem::path(sourceDirectory) / name);

		// Sanity:
		EXPECT_EQ(3u, test::CountFilesAndDirectories(sourceDirectory));
		EXPECT_EQ(0u, test::CountFilesAndDirectories(destDirectory));

		// Act:
		MoveAllFiles(sourceDirectory, destDirectory);

		// Assert:
		EXPECT_EQ(3u, test::CountFilesAndDirectories(sourceDirectory));
		EXPECT_EQ(0u, test::CountFilesAndDirectories(destDirectory));
	}

	TEST(TEST_CLASS, MoveAllFiles_MovesFiles) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto sourceDirectory = (std::filesystem::path(tempDir.name()) / "source").generic_string();
		auto destDirectory = (std::filesystem::path(tempDir.name()) / "dest").generic_string();

		std::filesystem::create_directory(sourceDirectory);
		std::filesystem::create_directory(destDirectory);

		// - create three files
		for (const auto& name : { "alpha", "beta", "gamma" })
			io::RawFile((std::filesystem::path(sourceDirectory) / name).generic_string(), io::OpenMode::Read_Write);

		// Sanity:
		EXPECT_EQ(3u, test::CountFilesAndDirectories(sourceDirectory));
		EXPECT_EQ(0u, test::CountFilesAndDirectories(destDirectory));

		// Act:
		MoveAllFiles(sourceDirectory, destDirectory);

		// Assert:
		EXPECT_EQ(0u, test::CountFilesAndDirectories(sourceDirectory));
		EXPECT_EQ(3u, test::CountFilesAndDirectories(destDirectory));
	}

	// endregion
}}

