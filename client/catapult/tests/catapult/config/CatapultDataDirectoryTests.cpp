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

#include "catapult/config/CatapultDataDirectory.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

#define TEST_CLASS CatapultDataDirectoryTests

	// region CatapultDirectory

	TEST(TEST_CLASS, CanCreateCatapultDirectory) {
		// Act:
		CatapultDirectory directory("foo/bar");

		// Assert:
		EXPECT_EQ("foo/bar", directory.str());
		EXPECT_EQ(boost::filesystem::path("foo/bar"), directory.path());
		EXPECT_EQ("foo/bar/abc", directory.file("abc"));
	}

	// endregion

	// region CatapultDataDirectory

	TEST(TEST_CLASS, CanGetRootDataDirectory) {
		// Arrange:
		CatapultDataDirectory dataDirectory("foo");

		// Act:
		auto directory = dataDirectory.rootDir();

		// Assert:
		EXPECT_EQ("foo", directory.str());
		EXPECT_EQ(boost::filesystem::path("foo"), directory.path());
		EXPECT_EQ("foo/abc", directory.file("abc"));
	}

	TEST(TEST_CLASS, CanGetSubDirectory) {
		// Arrange:
		CatapultDataDirectory dataDirectory("foo");

		// Act:
		auto directory = dataDirectory.dir("bar");

		// Assert:
		EXPECT_EQ("foo/bar", directory.str());
		EXPECT_EQ(boost::filesystem::path("foo/bar"), directory.path());
		EXPECT_EQ("foo/bar/abc", directory.file("abc"));
	}

	TEST(TEST_CLASS, CanGetSpoolSubDirectory) {
		// Arrange:
		CatapultDataDirectory dataDirectory("foo");

		// Act:
		auto directory = dataDirectory.spoolDir("bar");

		// Assert:
		EXPECT_EQ("foo/spool/bar", directory.str());
		EXPECT_EQ(boost::filesystem::path("foo/spool/bar"), directory.path());
		EXPECT_EQ("foo/spool/bar/abc", directory.file("abc"));
	}

	// endregion

	// region CatapultDataDirectoryPreparer

	namespace {
		constexpr const char* Sub_Directories[] = { "/spool" };
	}

	TEST(TEST_CLASS, PreparerCreatesSubDirectoriesWhenNotPresent) {
		// Arrange:
		test::TempDirectoryGuard tempDir;

		// Sanity:
		for (const auto* subDirectory : Sub_Directories)
			EXPECT_FALSE(boost::filesystem::is_directory(tempDir.name() + subDirectory)) << subDirectory;

		// Act:
		auto dataDirectory = CatapultDataDirectoryPreparer::Prepare(tempDir.name());

		// Assert:
		EXPECT_EQ(tempDir.name(), dataDirectory.rootDir().str());
		for (const auto* subDirectory : Sub_Directories)
			EXPECT_TRUE(boost::filesystem::is_directory(tempDir.name() + subDirectory)) << subDirectory;
	}

	TEST(TEST_CLASS, PreparerSucceedsWhenSubDirectoriesArePresent) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		for (const auto* subDirectory : Sub_Directories)
			boost::filesystem::create_directory(tempDir.name() + subDirectory);

		// Sanity:
		for (const auto* subDirectory : Sub_Directories)
			EXPECT_TRUE(boost::filesystem::is_directory(tempDir.name() + subDirectory)) << subDirectory;

		// Act:
		auto dataDirectory = CatapultDataDirectoryPreparer::Prepare(tempDir.name());

		// Assert:
		EXPECT_EQ(tempDir.name(), dataDirectory.rootDir().str());
		for (const auto* subDirectory : Sub_Directories)
			EXPECT_TRUE(boost::filesystem::is_directory(tempDir.name() + subDirectory)) << subDirectory;
	}

	// endregion
}}
