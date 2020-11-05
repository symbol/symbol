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
#include "catapult/io/RawFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

#define TEST_CLASS CatapultDataDirectoryTests

	namespace fs = boost::filesystem;

	namespace {
		struct TestKey_tag {};
		using TestKey = utils::BaseValue<uint32_t, TestKey_tag>;

		auto Concat(const fs::path& baseDirectory, const fs::path& subDirectory) {
			return baseDirectory / subDirectory;
		}

		void AssertPermissions(const fs::path& fullPath) {
			auto status = fs::status(fullPath);
			EXPECT_NE(fs::perms::perms_not_known, status.permissions());

			// permissions are not set on windows
#ifndef _WIN32
			EXPECT_EQ(fs::perms::owner_all, status.permissions() & fs::perms::all_all);
#endif
		}
	}

	// region CatapultDirectory - basic

	TEST(TEST_CLASS, CanCreateCatapultDirectory) {
		// Act:
		CatapultDirectory directory("foo/bar");

		// Assert:
		EXPECT_EQ("foo/bar", directory.str());
		EXPECT_EQ(fs::path("foo/bar"), directory.path());
		EXPECT_EQ("foo/bar/abc", directory.file("abc"));

		EXPECT_FALSE(directory.exists());
	}

	TEST(TEST_CLASS, ExistsReturnsFalseWhenDirectoryDoesNotExists) {
		// Arrange:
		CatapultDirectory directory("foo");

		// Act + Assert:
		EXPECT_FALSE(directory.exists());
	}

	TEST(TEST_CLASS, ExistsReturnsTrueWhenDirectoryExists) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		CatapultDirectory directory(tempDir.name());

		// Act + Assert:
		EXPECT_TRUE(directory.exists());
	}

	// endregion

	// region CatapultDirectory - filesystem

	namespace {
		struct CreateTraits {
			static void Create(const CatapultDirectory& directory) {
				directory.create();
			}
		};

		struct CreateAllTraits {
			static void Create(const CatapultDirectory& directory) {
				directory.createAll();
			}
		};
	}

#define CREATE_DIRECTORY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, CatapultDirectoryCreate_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CreateTraits>(); } \
	TEST(TEST_CLASS, CatapultDirectoryCreateAll_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CreateAllTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CREATE_DIRECTORY_TEST(CanCreateSingleDirectory_Filesystem) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto fullPath = Concat(tempDir.name(), "foobar");
		CatapultDirectory dataDirectory(fullPath);

		// Sanity:
		EXPECT_FALSE(dataDirectory.exists());

		// Act:
		TTraits::Create(dataDirectory);

		// Assert:
		EXPECT_TRUE(dataDirectory.exists());
		AssertPermissions(fullPath);
	}

	CREATE_DIRECTORY_TEST(SucceedsWhenDirectoryAlreadyExists_Filesystem) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto fullPath = Concat(tempDir.name(), "foobar");
		CatapultDirectory dataDirectory(fullPath);

		TTraits::Create(dataDirectory);

		// Sanity:
		EXPECT_TRUE(dataDirectory.exists());

		// Act:
		TTraits::Create(dataDirectory);

		// Assert:
		EXPECT_TRUE(dataDirectory.exists());
		AssertPermissions(fullPath);
	}

	CREATE_DIRECTORY_TEST(ThrowsWhenCollidingFileExists_Filesystem) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto fullPath = Concat(tempDir.name(), "foo");
		CatapultDirectory dataDirectory(fullPath);

		io::RawFile file(fullPath.generic_string(), io::OpenMode::Read_Write);

		// Sanity:
		EXPECT_TRUE(fs::exists(fullPath));
		EXPECT_FALSE(dataDirectory.exists());

		// Act + Assert:
		EXPECT_THROW(TTraits::Create(dataDirectory), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CatapultDirectoryCreate_ThrowsWhenDirectoryCannotBeCreated_Filesystem) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto fullPath = Concat(tempDir.name(), "foo") / "bar";
		CatapultDirectory dataDirectory(fullPath);

		// Sanity:
		EXPECT_FALSE(dataDirectory.exists());

		// Act + Assert:
		EXPECT_THROW(CatapultDirectory(fullPath).create(), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CatapultDirectoryCreateAll_CanCreateMultipleDirectories_Filesystem) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto fullPath = Concat(tempDir.name(), "foo") / "baz" / "bar";
		CatapultDirectory dataDirectory(fullPath);

		// Sanity:
		EXPECT_FALSE(dataDirectory.exists());

		// Act:
		CatapultDirectory(fullPath).createAll();

		// Assert:
		EXPECT_TRUE(dataDirectory.exists());
		AssertPermissions(Concat(tempDir.name(), "foo"));
		AssertPermissions(Concat(tempDir.name(), "foo") / "baz");
		AssertPermissions(Concat(tempDir.name(), "foo") / "baz" / "bar");
	}

	TEST(TEST_CLASS, CatapultDirectoryCreateAll_CanCreateMultipleDirectoriesWhenSomeAlreadyExist_Filesystem) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		auto partialPath = Concat(tempDir.name(), "foo") / "baz";
		CatapultDirectory partialDirectory(partialPath);
		auto fullPath = partialPath / "bar" / "moon";
		CatapultDirectory fullDirectory(fullPath);

		partialDirectory.createAll();

		// Sanity:
		EXPECT_TRUE(partialDirectory.exists());
		EXPECT_FALSE(fullDirectory.exists());

		// Act:
		fullDirectory.createAll();

		// Assert:
		EXPECT_TRUE(partialDirectory.exists());
		EXPECT_TRUE(fullDirectory.exists());
		AssertPermissions(Concat(tempDir.name(), "foo"));
		AssertPermissions(Concat(tempDir.name(), "foo") / "baz");
		AssertPermissions(Concat(tempDir.name(), "foo") / "baz" / "bar");
		AssertPermissions(Concat(tempDir.name(), "foo") / "baz" / "bar" / "moon");
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
		EXPECT_EQ(fs::path("foo"), directory.path());
		EXPECT_EQ("foo/abc", directory.file("abc"));
	}

	TEST(TEST_CLASS, CanGetSubDirectory) {
		// Arrange:
		CatapultDataDirectory dataDirectory("foo");

		// Act:
		auto directory = dataDirectory.dir("bar");

		// Assert:
		EXPECT_EQ("foo/bar", directory.str());
		EXPECT_EQ(fs::path("foo/bar"), directory.path());
		EXPECT_EQ("foo/bar/abc", directory.file("abc"));
	}

	TEST(TEST_CLASS, CanGetSpoolSubDirectory) {
		// Arrange:
		CatapultDataDirectory dataDirectory("foo");

		// Act:
		auto directory = dataDirectory.spoolDir("bar");

		// Assert:
		EXPECT_EQ("foo/spool/bar", directory.str());
		EXPECT_EQ(fs::path("foo/spool/bar"), directory.path());
		EXPECT_EQ("foo/spool/bar/abc", directory.file("abc"));
	}

	TEST(TEST_CLASS, CanGetStorageDirectory) {
		// Arrange:
		CatapultDataDirectory dataDirectory("foo");

		// Act:
		auto storageDirectory = dataDirectory.storageDir(TestKey(Files_Per_Storage_Directory * 5 + 13));

		// Assert:
		auto subDirectoryPath = Concat("foo", "00005");
		EXPECT_EQ(subDirectoryPath.generic_string(), storageDirectory.str());
		EXPECT_EQ((subDirectoryPath / "00013:suffix").generic_string(), storageDirectory.storageFile(":suffix"));
		EXPECT_EQ((subDirectoryPath / ":suffix").generic_string(), storageDirectory.indexFile(std::string(), ":suffix"));
		EXPECT_EQ((subDirectoryPath / "prefix_:suffix").generic_string(), storageDirectory.indexFile("prefix_", ":suffix"));
	}

	// endregion

	// region CatapultDataDirectoryPreparer

	namespace {
		constexpr const char* Sub_Directories[] = { "/importance", "/spool" };
	}

	TEST(TEST_CLASS, CatapultDataDirectoryPreparer_PreparerCreatesSubDirectoriesWhenNotPresent) {
		// Arrange:
		test::TempDirectoryGuard tempDir;

		// Sanity:
		for (const auto* subDirectory : Sub_Directories)
			EXPECT_FALSE(fs::is_directory(tempDir.name() + subDirectory)) << subDirectory;

		// Act:
		auto dataDirectory = CatapultDataDirectoryPreparer::Prepare(tempDir.name());

		// Assert:
		EXPECT_EQ(tempDir.name(), dataDirectory.rootDir().str());
		for (const auto* subDirectory : Sub_Directories)
			EXPECT_TRUE(fs::is_directory(tempDir.name() + subDirectory)) << subDirectory;

		EXPECT_EQ(CountOf(Sub_Directories), test::CountFilesAndDirectories(tempDir.name()));
	}

	TEST(TEST_CLASS, CatapultDataDirectoryPreparer_PreparerSucceedsWhenSubDirectoriesArePresent) {
		// Arrange:
		test::TempDirectoryGuard tempDir;
		for (const auto* subDirectory : Sub_Directories)
			fs::create_directory(tempDir.name() + subDirectory);

		// Sanity:
		for (const auto* subDirectory : Sub_Directories)
			EXPECT_TRUE(fs::is_directory(tempDir.name() + subDirectory)) << subDirectory;

		// Act:
		auto dataDirectory = CatapultDataDirectoryPreparer::Prepare(tempDir.name());

		// Assert:
		EXPECT_EQ(tempDir.name(), dataDirectory.rootDir().str());
		for (const auto* subDirectory : Sub_Directories)
			EXPECT_TRUE(fs::is_directory(tempDir.name() + subDirectory)) << subDirectory;

		EXPECT_EQ(CountOf(Sub_Directories), test::CountFilesAndDirectories(tempDir.name()));
	}

	// endregion

	// region CatapultStorageDirectoryPreparer

	namespace {
		void AssertPrepareStorageCreatesSubDirectory(TestKey key, const std::string& expectedSubDirectoryName) {
			// Arrange:
			test::TempDirectoryGuard tempDir;

			// Sanity:
			EXPECT_FALSE(fs::exists(Concat(tempDir.name(), expectedSubDirectoryName)));

			// Act:
			auto storageDirectory = config::CatapultStorageDirectoryPreparer::Prepare(tempDir.name(), key);

			// Assert:
			EXPECT_TRUE(fs::exists(Concat(tempDir.name(), expectedSubDirectoryName)));
		}
	}

	TEST(TEST_CLASS, CatapultStorageDirectoryPreparer_PreparerCreatesDirectory) {
		AssertPrepareStorageCreatesSubDirectory(TestKey(5), "00000");
	}

	TEST(TEST_CLASS, CatapultStorageDirectoryPreparer_PreparerCreatesDirectory_BeforeBoundary) {
		AssertPrepareStorageCreatesSubDirectory(TestKey(Files_Per_Storage_Directory - 1), "00000");
	}

	TEST(TEST_CLASS, CatapultStorageDirectoryPreparer_PreparerCreatesDirectory_AtBoundary) {
		AssertPrepareStorageCreatesSubDirectory(TestKey(Files_Per_Storage_Directory), "00001");
	}

	TEST(TEST_CLASS, CatapultStorageDirectoryPreparer_PreparerCreatesDirectory_AfterBoundary) {
		AssertPrepareStorageCreatesSubDirectory(TestKey(Files_Per_Storage_Directory + 1), "00001");
	}

	// endregion
}}
