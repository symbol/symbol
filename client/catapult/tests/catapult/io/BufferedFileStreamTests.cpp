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

#include "catapult/io/BufferedFileStream.h"
#include "tests/catapult/io/test/StreamTests.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS BufferedFileStreamTests

	namespace {
		constexpr auto Default_Test_Buffer_Size = test::StreamTests<void>::Default_Test_Buffer_Size;

		class BufferedFileStreamContext {
		public:
			explicit BufferedFileStreamContext(const char* name) : m_guard(name)
			{}

			auto outputStream() const {
				return MakeStream<BufferedOutputFileStream>(OpenMode::Read_Write);
			}

			auto inputStream() const {
				return MakeStream<BufferedInputFileStream>(OpenMode::Read_Only);
			}

			std::string filename() const {
				return m_guard.name();
			}

		private:
			template<typename T>
			std::unique_ptr<T> MakeStream(OpenMode mode) const {
				return std::make_unique<T>(RawFile(filename(), mode), Default_Test_Buffer_Size);
			}

		private:
			test::TempFileGuard m_guard;
		};

		class ReadWriteTest : BufferedFileStreamContext {
		public:
			ReadWriteTest() : BufferedFileStreamContext("test.dat")
			{}

		public:
			void write(std::initializer_list<size_t> writeSizes, size_t chunkSize = 0) {
				auto pOutput = outputStream();
				writeToOutput(*pOutput, writeSizes, chunkSize);
				// NOTE: there is no flush done after writing
			}

			void rawWrite(std::initializer_list<size_t> writeSizes, size_t chunkSize = 0) {
				RawFile file(filename(), OpenMode::Read_Write);
				writeToOutput(file, writeSizes, chunkSize);
			}

		public:
			void assertReadThrows(size_t expectedSize, size_t readSize) const {
				// Assert: the file size
				RawFile file(filename(), OpenMode::Read_Only);
				auto actualSize = file.size();
				ASSERT_EQ(expectedSize, actualSize);

				// - the read fails
				std::vector<uint8_t> result(readSize);
				BufferedInputFileStream input(std::move(file), Default_Test_Buffer_Size);
				EXPECT_THROW(input.read(result), catapult_file_io_error);
			}

			void assertRead(size_t expectedSize, std::initializer_list<size_t> readSizes) const {
				// Assert: the file size
				RawFile file(filename(), OpenMode::Read_Only);
				auto actualSize = file.size();
				ASSERT_EQ(expectedSize, actualSize);

				// - the file contents
				BufferedInputFileStream input(std::move(file), Default_Test_Buffer_Size);
				auto i = 0u;
				auto expectedIter = m_expected.begin();
				for (auto readSize : readSizes) {
					std::vector<uint8_t> result(readSize);
					input.read(result);

					using difference_type = std::vector<uint8_t>::difference_type;
					std::vector<uint8_t> expected(expectedIter, expectedIter + static_cast<difference_type>(readSize));
					EXPECT_EQ(expected, result) << "failed during " << ++i << "th read";
					expectedIter += static_cast<difference_type>(readSize);
				}
			}

		private:
			template<typename TOutput>
			void writeToOutput(TOutput& output, std::initializer_list<size_t> writeSizes, size_t chunkSize) {
				for (auto writeSize : writeSizes) {
					auto randomData = test::GenerateRandomVector(writeSize);
					CATAPULT_LOG(info) << "writing " << randomData[0] << " " << randomData[1];

					auto currentChunkSize = 0 == chunkSize ? writeSize : chunkSize;
					for (size_t i = 0u; i < writeSize; i += currentChunkSize)
						output.write({ randomData.data() + i, std::min(currentChunkSize, writeSize - i) });

					m_expected.insert(m_expected.end(), randomData.cbegin(), randomData.cend());
				}
			}

		private:
			std::vector<uint8_t> m_expected;
		};
	}

	// region write/flush tests

	TEST(TEST_CLASS, WritingLessThanBufferSizeDoesNotFlush) {
		// Arrange:
		ReadWriteTest test;

		// Act: no flush happened, so we shouldn't be able to read anything
		test.write({ Default_Test_Buffer_Size - 1 });

		// Assert:
		test.assertRead(0, {});
	}

	TEST(TEST_CLASS, WritingEqualToBufferSizeFlushes) {
		// Arrange
		ReadWriteTest test;

		// Act: whole buffer has been written in one write, so we should be able to read everything
		test.write({ Default_Test_Buffer_Size });

		// Assert:
		test.assertRead(Default_Test_Buffer_Size, { Default_Test_Buffer_Size });
	}

	TEST(TEST_CLASS, WritingMoreThanBufferSizeFlushes) {
		// Arrange
		constexpr auto Data_Size = 3 * Default_Test_Buffer_Size - 100;
		ReadWriteTest test;

		// Act: whole buffer has been written in one write, so we should be able to read everything
		test.write({ Data_Size });

		// Assert:
		test.assertRead(Data_Size, { Data_Size });
	}

	TEST(TEST_CLASS, WritingChunkedMoreThanBufferSizeDoesNotFlushLastBuffer) {
		// Arrange
		constexpr auto Data_Size = 3 * Default_Test_Buffer_Size - 100;
		ReadWriteTest test;

		// Act: write in 100 byte chunks (so only two flushes)
		test.write({ Data_Size }, 100);

		// Assert:
		constexpr auto Expected_File_Size = 2 * Default_Test_Buffer_Size;
		test.assertRead(Expected_File_Size, { Expected_File_Size });
	}

	TEST(TEST_CLASS, ShortWriteFollowedByEqualWriteFlushesTheData) {
		// Arrange:
		ReadWriteTest test;

		// Act: second write causes flush, but last 100 bytes won't be flushed
		test.write({ 100, Default_Test_Buffer_Size });

		// Assert:
		constexpr auto Expected_File_Size = Default_Test_Buffer_Size;
		test.assertRead(Expected_File_Size, { Expected_File_Size });
	}

	TEST(TEST_CLASS, ShortWriteFollowedByLargeWriteFlushesTheData) {
		// Arrange:
		ReadWriteTest test;

		// Act: second write causes flush of everything
		test.write({ 100, 2 * Default_Test_Buffer_Size });

		// Assert:
		constexpr auto Expected_File_Size = 2 * Default_Test_Buffer_Size + 100;
		test.assertRead(Expected_File_Size, { Expected_File_Size });
	}

	TEST(TEST_CLASS, ShortWriteFollowedByFillingShortFlushesTheData) {
		// Arrange:
		ReadWriteTest test;

		// Act: second write causes flush of everything
		test.write({ 100, Default_Test_Buffer_Size - 100 });

		// Assert:
		constexpr auto Expected_File_Size = Default_Test_Buffer_Size;
		test.assertRead(Expected_File_Size, { Expected_File_Size });
	}

	TEST(TEST_CLASS, ShortWriteFollowedByEqualFollowedByFillingShortFlushesTheData) {
		// Arrange:
		ReadWriteTest test;

		// Act: first part is same as in ShortWriteFollowedByEqualWriteFlushesTheData
		//      then we add filling short, to flush 2 x Default_Test_Buffer_Size
		test.write({ 100, Default_Test_Buffer_Size, Default_Test_Buffer_Size - 100 });

		// Assert:
		constexpr auto Expected_File_Size = 2 * Default_Test_Buffer_Size;
		test.assertRead(Expected_File_Size, { Expected_File_Size });
	}

	// endregion

	// region read tests

	TEST(TEST_CLASS, ReadingLessThanBufferSize_FileLarger) {
		// Arrange:
		constexpr auto Data_Size = 3 * Default_Test_Buffer_Size;
		ReadWriteTest test;
		test.rawWrite({ Data_Size });

		// Act + Assert:
		test.assertRead(Data_Size, { Default_Test_Buffer_Size - 1 });
	}

	TEST(TEST_CLASS, ReadingLessThanBufferSize_FileEqual) {
		// Arrange:
		constexpr auto Data_Size = Default_Test_Buffer_Size - 1;
		ReadWriteTest test;
		test.rawWrite({ Data_Size });

		// Act + Assert:
		test.assertRead(Data_Size, { Default_Test_Buffer_Size - 1 });
	}

	TEST(TEST_CLASS, ReadingLessThanBufferSize_FileSmaller_Throws) {
		// Arrange:
		constexpr auto Data_Size = Default_Test_Buffer_Size - 2;
		ReadWriteTest test;
		test.rawWrite({ Data_Size });

		// Act + Assert:
		test.assertReadThrows(Data_Size, Default_Test_Buffer_Size - 1);
	}

	TEST(TEST_CLASS, ReadingLessThanBufferSize_Multiple) {
		// Arrange:
		constexpr auto Data_Size = 2 * Default_Test_Buffer_Size;
		ReadWriteTest test;
		test.rawWrite({ Data_Size });

		// Act + Assert:
		test.assertRead(Data_Size, { Default_Test_Buffer_Size - 1, Default_Test_Buffer_Size - 1, 2 });
	}

	TEST(TEST_CLASS, ReadingLessThanBufferSize_FollowedByLargeRead) {
		// Arrange:
		constexpr auto Data_Size = 2 * Default_Test_Buffer_Size;
		ReadWriteTest test;
		test.rawWrite({ Data_Size });

		// Act + Assert:
		test.assertRead(Data_Size, { 10, 2 * Default_Test_Buffer_Size - 10 });
	}

	TEST(TEST_CLASS, ReadingLessThanBufferSize_CrossBufferSizeBoundary) {
		// Arrange:
		constexpr auto Data_Size = 2 * Default_Test_Buffer_Size;
		ReadWriteTest test;
		test.rawWrite({ Data_Size });

		// Act + Assert: (second read crosses buffer boundary)
		test.assertRead(Data_Size, { Default_Test_Buffer_Size - 10, 20, Default_Test_Buffer_Size - 10 });
	}

	TEST(TEST_CLASS, ReadingMoreThanBufferSize_FollowedBySmall) {
		// Arrange:
		constexpr auto Data_Size = 2 * Default_Test_Buffer_Size;
		ReadWriteTest test;
		test.rawWrite({ Data_Size });

		// Act + Assert:
		test.assertRead(Data_Size, { 2 * Default_Test_Buffer_Size - 10, 10 });
	}

	TEST(TEST_CLASS, ReadingEqualToBufferSize_FollowedBySmall) {
		// Arrange:
		constexpr auto Data_Size = 2 * Default_Test_Buffer_Size;
		ReadWriteTest test;
		test.rawWrite({ Data_Size });

		// Act + Assert:
		constexpr auto Small = Default_Test_Buffer_Size / 2;
		test.assertRead(Data_Size, { Default_Test_Buffer_Size, Small, Default_Test_Buffer_Size - Small });
	}

	// endregion

	DEFINE_STREAM_TESTS(BufferedFileStreamContext)
}}
