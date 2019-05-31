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

#include "catapult/io/FilesystemUtils.h"
#include "catapult/io/RawFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace io {

#define TEST_CLASS FilesystemUtilsTests

	TEST(TEST_CLASS, PurgeDirectory_HasNoEffectOnNonexistentDirectory) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto directoryPath = boost::filesystem::path(tempDir.name()) / "staging";

		// Sanity:
		EXPECT_FALSE(boost::filesystem::exists(directoryPath));

		// Act:
		PurgeDirectory(directoryPath.generic_string());

		// Assert:
		EXPECT_FALSE(boost::filesystem::exists(directoryPath));
	}

	TEST(TEST_CLASS, PurgeDirectory_HasNoEffectOnEmptyDirectory) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto directoryPath = boost::filesystem::path(tempDir.name()) / "staging";
		boost::filesystem::create_directory(directoryPath);

		// Sanity:
		EXPECT_TRUE(boost::filesystem::exists(directoryPath));

		// Act:
		PurgeDirectory(directoryPath.generic_string());

		// Assert:
		EXPECT_TRUE(boost::filesystem::exists(directoryPath));
		EXPECT_EQ(0u, test::CountFilesAndDirectories(directoryPath));
	}

	TEST(TEST_CLASS, PurgeDirectory_RemovesAllSubFiles) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto directoryPath = boost::filesystem::path(tempDir.name()) / "staging";
		boost::filesystem::create_directory(directoryPath);

		// - create three files
		for (const auto& name : { "alpha", "beta", "gamma" })
			io::RawFile((directoryPath / name).generic_string(), io::OpenMode::Read_Write);

		// Sanity:
		EXPECT_EQ(3u, test::CountFilesAndDirectories(directoryPath));

		// Act:
		PurgeDirectory(directoryPath.generic_string());

		// Assert:
		EXPECT_TRUE(boost::filesystem::exists(directoryPath));
		EXPECT_EQ(0u, test::CountFilesAndDirectories(directoryPath));
	}

	TEST(TEST_CLASS, PurgeDirectory_RemovesAllSubFolders) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto directoryPath = boost::filesystem::path(tempDir.name()) / "staging";
		boost::filesystem::create_directory(directoryPath);

		// - create three files inside of a subdirectory
		boost::filesystem::create_directory(directoryPath / "sub");
		for (const auto& name : { "aaa", "bbb", "ccc" })
			io::RawFile((directoryPath / "sub" / name).generic_string(), io::OpenMode::Read_Write);

		// Sanity:
		EXPECT_EQ(1u, test::CountFilesAndDirectories(directoryPath));
		EXPECT_EQ(3u, test::CountFilesAndDirectories(directoryPath / "sub"));

		// Act:
		PurgeDirectory(directoryPath.generic_string());

		// Assert:
		EXPECT_TRUE(boost::filesystem::exists(directoryPath));
		EXPECT_EQ(0u, test::CountFilesAndDirectories(directoryPath));

		EXPECT_FALSE(boost::filesystem::exists(directoryPath / "sub"));
	}
}}

