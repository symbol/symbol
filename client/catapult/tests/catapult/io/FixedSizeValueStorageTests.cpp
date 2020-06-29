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

#include "catapult/io/FixedSizeValueStorage.h"
#include "catapult/io/BufferedFileStream.h"
#include "catapult/constants.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace catapult { namespace io {

#define TEST_CLASS FixedSizeValueStorageTests

	// region test utils

	namespace {
		using ValueType = Hash256;

		ValueType ToValue(uint64_t seed) {
			ValueType taggedValue;
			auto rawTag = seed * 3 + 1;
			std::memcpy(taggedValue.data(), &rawTag, sizeof(uint64_t));
			return taggedValue;
		}

		class TestContext {
		public:
			TestContext() : m_hashFile(m_guard.name(), "test")
			{}

		public:
			auto filename(const std::string& subDirectoryName) const {
				return (fs::path(m_guard.name()) / subDirectoryName / "test.dat").generic_string();
			}

			auto storageExists(const std::string& subDirectoryName) const {
				auto subDirectoryPath = fs::path(m_guard.name()) / subDirectoryName;
				auto filePath = subDirectoryPath / "test.dat";

				return boost::filesystem::exists(subDirectoryPath) && boost::filesystem::exists(filePath);
			}

			auto& hashFile() {
				return m_hashFile;
			}

		public:
			void seed(size_t numValues) {
				auto fileIndex = 0;
				size_t valuesWritten = 0;
				while (numValues) {
					auto subDirectoryName = std::string("0000") + static_cast<char>('0' + fileIndex);
					auto count = std::min<size_t>(Files_Per_Storage_Directory, numValues);
					seedFile(subDirectoryName, valuesWritten, count);

					valuesWritten = valuesWritten + count;
					numValues = numValues - count;
					++fileIndex;
				}
			}

		private:
			void seedFile(const std::string& subDirectoryName, size_t offset, size_t numValues) {
				fs::create_directory(fs::path(m_guard.name()) / subDirectoryName);

				BufferedOutputFileStream stream(RawFile(filename(subDirectoryName), OpenMode::Read_Write));
				for (auto i = 0u; i < numValues; ++i)
					stream.write(ToValue(offset + i));

				stream.flush();
			}

		private:
			test::TempDirectoryGuard m_guard;
			HashFile m_hashFile;
		};
	}

	// endregion

	// region lazy open

	TEST(TEST_CLASS, StorageIsLazilyOpened) {
		// Arrange:
		TestContext context;
		auto value = test::GenerateRandomByteArray<ValueType>();

		// Sanity: file is not opened at this point
		EXPECT_FALSE(context.storageExists("00001"));

		// Act:
		context.hashFile().save(Height(Files_Per_Storage_Directory), value);

		// Assert:
		EXPECT_TRUE(context.storageExists("00001"));
		EXPECT_EQ(ValueType::Size, boost::filesystem::file_size(context.filename("00001")));
	}

	TEST(TEST_CLASS, StorageCannotOpenWithoutAnyValues_Save) {
		// Arrange:
		TestContext context;
		auto value = test::GenerateRandomByteArray<ValueType>();

		// Act + Assert:
		EXPECT_THROW(context.hashFile().save(Height(0), value), catapult_runtime_error);
	}

	TEST(TEST_CLASS, StorageCannotOpenWithoutTwoValues_Save) {
		// Arrange:
		TestContext context;
		context.seed(1);
		auto value = test::GenerateRandomByteArray<ValueType>();

		// Act + Assert:
		EXPECT_THROW(context.hashFile().save(Height(1), value), catapult_runtime_error);
	}

	TEST(TEST_CLASS, StorageCannotOpenWithoutTwoValues_Load) {
		// Arrange: prepare storage with single value
		TestContext context;
		context.seed(1);

		// Act + Assert:
		EXPECT_THROW(context.hashFile().loadRangeFrom(Height(0), 1), catapult_runtime_error);
	}

	// endregion

	// region loading

	namespace {
		void AssertValue(uint64_t key, const ValueType& value) {
			auto expectedValue = ToValue(key);
			EXPECT_EQ(expectedValue, value);
		}

		void AssertValues(const model::EntityRange<ValueType>& values, std::initializer_list<uint64_t> expectedValues) {
			ASSERT_EQ(expectedValues.size(), values.size());

			auto iter = values.begin();
			for (auto expectedValue : expectedValues)
				AssertValue(expectedValue, *iter++);
		}

		void AssertValues(const model::EntityRange<ValueType>& values, uint64_t startKey, size_t expectedSize) {
			ASSERT_EQ(expectedSize, values.size());
			for (const auto& value : values)
				AssertValue(startKey++, value);
		}
	}

	TEST(TEST_CLASS, StorageCanLoadSingleValue) {
		// Arrange:
		TestContext context;
		context.seed(75'536);

		// Act:
		auto firstRange = context.hashFile().loadRangeFrom(Height(0), 1);
		auto middleRange = context.hashFile().loadRangeFrom(Height(30'000), 1);
		auto lastRange = context.hashFile().loadRangeFrom(Height(75'535), 1);

		// Assert:
		AssertValues(firstRange, { 0 });
		AssertValues(middleRange, { 30'000 });
		AssertValues(lastRange, { 75'535 });
	}

	TEST(TEST_CLASS, StorageCanLoadMultipleValues) {
		// Arrange:
		TestContext context;
		context.seed(20);

		// Act:
		auto values = context.hashFile().loadRangeFrom(Height(5), 10);

		// Assert:
		AssertValues(values, 5, 10);
	}

	TEST(TEST_CLASS, StorageCanLoadValuesSpanningMultipleFiles) {
		// Arrange: seed at least 3 files
		TestContext context;
		context.seed(Files_Per_Storage_Directory * 2 + 10'000);

		// Sanity:
		EXPECT_TRUE(context.storageExists("00000"));
		EXPECT_TRUE(context.storageExists("00001"));
		EXPECT_TRUE(context.storageExists("00002"));

		// Act: read values spanning multiple files
		auto values = context.hashFile().loadRangeFrom(Height(Files_Per_Storage_Directory - 5), Files_Per_Storage_Directory + 10);

		// Assert:
		AssertValues(values, Files_Per_Storage_Directory - 5, Files_Per_Storage_Directory + 10);
	}

	// endregion

	// region saving

	TEST(TEST_CLASS, StorageCanSaveAscendingKeys) {
		// Arrange:
		TestContext context;
		context.seed(2);

		// Act:
		context.hashFile().save(Height(2), ToValue(12));
		context.hashFile().save(Height(3), ToValue(13));
		context.hashFile().save(Height(4), ToValue(14));

		auto values = context.hashFile().loadRangeFrom(Height(2), 3);

		// Assert:
		ASSERT_EQ(5 * ValueType::Size, boost::filesystem::file_size(context.filename("00000")));
		AssertValues(values, { 12, 13, 14 });
	}

	TEST(TEST_CLASS, StorageCannotSaveSkippingSomeKeys) {
		// Arrange:
		TestContext context;
		context.seed(2);

		// Act + Assert:
		EXPECT_THROW(context.hashFile().save(Height(4), ToValue(14)), catapult_file_io_error);
	}

	TEST(TEST_CLASS, StorageCanOverwriteExistingKeys) {
		// Arrange:
		TestContext context;
		context.seed(5);

		// Act:
		context.hashFile().save(Height(3), ToValue(13));
		context.hashFile().save(Height(2), ToValue(12));

		auto values = context.hashFile().loadRangeFrom(Height(1), 4);

		// Assert: values at heights 1 and 4 come from seed, values at heights 2 and 3 are different (were overwritten)
		AssertValues(values, { 1, 12, 13, 4 });
	}

	// endregion
}}
