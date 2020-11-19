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

#include "catapult/io/IndexFile.h"
#include "catapult/io/RawFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <filesystem>

namespace catapult { namespace io {

#define TEST_CLASS IndexFileTests

	namespace {
		// region asserts

		void AssertNotExists(const test::TempFileGuard& tempFile, const IndexFile& indexFile) {
			// Assert:
			EXPECT_FALSE(std::filesystem::exists(tempFile.name()));

			EXPECT_FALSE(indexFile.exists());
		}

		void AssertExists(const test::TempFileGuard& tempFile, const IndexFile& indexFile) {
			// Assert:
			EXPECT_TRUE(std::filesystem::exists(tempFile.name()));

			EXPECT_TRUE(indexFile.exists());
			EXPECT_EQ(sizeof(uint64_t), RawFile(tempFile.name(), OpenMode::Read_Only).size());
		}

		// endregion
	}

	// region constructor / get

	TEST(TEST_CLASS, ConstructorDoesNotCreateFile) {
		// Act:
		test::TempFileGuard tempFile("foo.dat");
		IndexFile indexFile(tempFile.name());

		// Assert:
		AssertNotExists(tempFile, indexFile);
	}

	TEST(TEST_CLASS, GetDoesNotCreateFile) {
		// Arrange:
		test::TempFileGuard tempFile("foo.dat");
		IndexFile indexFile(tempFile.name());

		// Act:
		EXPECT_THROW(indexFile.get(), catapult_runtime_error);

		// Assert:
		AssertNotExists(tempFile, indexFile);
	}

	namespace {
		void AssertGetReturnsExpectedValue(uint64_t fileSize, uint64_t expectedValue) {
			// Arrange:
			test::TempFileGuard tempFile("foo.dat");
			IndexFile indexFile(tempFile.name());
			{
				RawFile rawFile(tempFile.name(), OpenMode::Read_Write);
				if (0u < fileSize)
					rawFile.write(std::vector<uint8_t>(fileSize, 1));

				// Sanity:
				EXPECT_EQ(fileSize, rawFile.size());
			}

			// Act:
			auto value = indexFile.get();

			// Assert:
			EXPECT_EQ(expectedValue, value) << "for file size " << fileSize;
		}
	}

	TEST(TEST_CLASS, GetReturnsZeroWhenFileSizeIsZero) {
		AssertGetReturnsExpectedValue(0, 0);
	}

	TEST(TEST_CLASS, GetReturnsZeroWhenFileSizeIsNeitherZeroNorEight) {
		AssertGetReturnsExpectedValue(1, 0);
		AssertGetReturnsExpectedValue(5, 0);
		AssertGetReturnsExpectedValue(7, 0);
		AssertGetReturnsExpectedValue(13, 0);
	}

	TEST(TEST_CLASS, GetReturnsCorrectValueWhenFileSizeIsEight) {
		AssertGetReturnsExpectedValue(8, 0x0101010101010101);
	}

	// endregion

	// region set

	TEST(TEST_CLASS, CanSetValue) {
		// Arrange:
		test::TempFileGuard tempFile("foo.dat");
		IndexFile indexFile(tempFile.name());

		// Act:
		indexFile.set(1234);

		// Assert:
		AssertExists(tempFile, indexFile);
		EXPECT_EQ(1234u, indexFile.get());
	}

	TEST(TEST_CLASS, CanResetValue) {
		// Arrange:
		test::TempFileGuard tempFile("foo.dat");
		IndexFile indexFile(tempFile.name());

		// Act:
		indexFile.set(1234);
		indexFile.set(87);

		// Assert:
		AssertExists(tempFile, indexFile);
		EXPECT_EQ(87u, indexFile.get());
	}

	// endregion

	// region increment

	TEST(TEST_CLASS, CanIncrementNewFileToZero) {
		// Arrange:
		test::TempFileGuard tempFile("foo.dat");
		IndexFile indexFile(tempFile.name());

		// Act:
		auto value = indexFile.increment();

		// Assert:
		AssertExists(tempFile, indexFile);
		EXPECT_EQ(0u, value);
		EXPECT_EQ(0u, indexFile.get());
	}

	TEST(TEST_CLASS, CanIncrementExistingFile) {
		// Arrange:
		test::TempFileGuard tempFile("foo.dat");
		IndexFile indexFile(tempFile.name());
		indexFile.set(1234);

		// Act:
		auto value = indexFile.increment();

		// Assert:
		AssertExists(tempFile, indexFile);
		EXPECT_EQ(1235u, value);
		EXPECT_EQ(1235u, indexFile.get());
	}

	// endregion

	// region file locking

	namespace {
		LockMode GetLockModeAllowingUnlockedAccess() {
			// returns lock mode that blocks LockMode::File but not LockMode::None
#ifdef _WIN32
			// on Windows, file locks are enforced proactively so LockMode::File will block all other file access
			return LockMode::None;
#else
			return LockMode::File;
#endif
		}
	}

	// note: a similar GetRespectsFileLocking test would require multithreading and be non-deterministic
	// because it only prevents aquisition of exclusive lock during short time when value is read from input file.
	// additionally, this behavior is better suited for and implicitly tested in integrity tests.

	TEST(TEST_CLASS, SetRespectsFileLocking) {
		// Arrange: create index files with different lock modes
		test::TempFileGuard tempFile("foo.dat");
		IndexFile indexFile1(tempFile.name());
		IndexFile indexFile2(tempFile.name(), LockMode::None);
		IndexFile indexFile3(tempFile.name(), LockMode::File);
		indexFile1.set(0);

		// - create a shared file lock
		RawFile fileLock(tempFile.name(), OpenMode::Read_Only, GetLockModeAllowingUnlockedAccess());

		// Act + Assert: index files with locking fail because they cannot obtain exclusive lock
		EXPECT_THROW(indexFile1.set(2), catapult_runtime_error);
		EXPECT_NO_THROW(indexFile2.set(2));
		EXPECT_THROW(indexFile3.set(2), catapult_runtime_error);
	}

	TEST(TEST_CLASS, IncrementRespectsFileLocking) {
		// Arrange: create index files with different lock modes
		test::TempFileGuard tempFile("foo.dat");
		IndexFile indexFile1(tempFile.name());
		IndexFile indexFile2(tempFile.name(), LockMode::None);
		IndexFile indexFile3(tempFile.name(), LockMode::File);
		indexFile1.set(0);

		// - create a shared file lock
		RawFile fileLock(tempFile.name(), OpenMode::Read_Only, GetLockModeAllowingUnlockedAccess());

		// Act + Assert: index files with locking fail because they cannot obtain exclusive lock
		EXPECT_THROW(indexFile1.increment(), catapult_runtime_error);
		EXPECT_NO_THROW(indexFile2.increment());
		EXPECT_THROW(indexFile3.increment(), catapult_runtime_error);
	}

	// endregion
}}
