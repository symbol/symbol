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

#include "catapult/io/FileQueue.h"
#include "catapult/io/IndexFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <filesystem>

namespace catapult { namespace io {

#define TEST_CLASS FileQueueTests

	namespace {
		// region directory traits

		struct DefaultTraits {
			static constexpr auto Index_Writer_Filename = "index.dat";
			static constexpr auto Index_Reader_Filename = "index_reader.dat";

			static FileQueueWriter CreateWriter(const std::string& directory) {
				return FileQueueWriter(directory);
			}

			static FileQueueReader CreateReader(const std::string& directory) {
				return FileQueueReader(directory);
			}
		};

		struct CustomTraits {
			static constexpr auto Index_Writer_Filename = "alpha.zzz";
			static constexpr auto Index_Reader_Filename = "beta.yyy";

			static FileQueueWriter CreateWriter(const std::string& directory) {
				return FileQueueWriter(directory, Index_Writer_Filename);
			}

			static FileQueueReader CreateReader(const std::string& directory) {
				return FileQueueReader(directory, Index_Reader_Filename, Index_Writer_Filename);
			}
		};

#define DIRECTORY_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Default) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DefaultTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Custom) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CustomTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		// endregion

		// region BasicQueueTestContext

		template<typename TTraits>
		class BasicQueueTestContext {
		public:
			explicit BasicQueueTestContext(const std::filesystem::path& directory)
					: m_tempDataDir(directory.generic_string())
					, m_directory(m_tempDataDir.name())
			{}

		public:
			const std::filesystem::path& directory() {
				return m_directory;
			}

		public:
			uint64_t readIndexWriterFile() const {
				return IndexFile((m_directory / TTraits::Index_Writer_Filename).generic_string()).get();
			}

			uint64_t readIndexReaderFile() const {
				return IndexFile((m_directory / TTraits::Index_Reader_Filename).generic_string()).get();
			}

			size_t countFiles() const {
				auto begin = std::filesystem::directory_iterator(m_directory);
				auto end = std::filesystem::directory_iterator();
				return static_cast<size_t>(std::distance(begin, end));
			}

			bool exists(const std::string& name) const {
				return std::filesystem::exists(m_directory / name);
			}

			std::vector<uint8_t> readAll(const std::string& name) {
				RawFile dataFile((m_directory / name).generic_string(), OpenMode::Read_Only);
				std::vector<uint8_t> buffer(dataFile.size());
				dataFile.read(buffer);
				return buffer;
			}

		private:
			test::TempDirectoryGuard m_tempDataDir;
			std::filesystem::path m_directory;
		};

		// endregion

		// region utils

		template<typename TTraits>
		void SetIndexes(const std::filesystem::path& directory, uint64_t indexWriterValue, uint64_t indexReaderValue) {
			if (0 != indexWriterValue)
				IndexFile((directory / TTraits::Index_Writer_Filename).generic_string()).set(indexWriterValue);

			if (0 != indexReaderValue)
				IndexFile((directory / TTraits::Index_Reader_Filename).generic_string()).set(indexReaderValue);
		}

		template<typename TTraits>
		void CreateDirectory(const std::string& name, uint64_t indexWriterValue, uint64_t indexReaderValue) {
			// Arrange:
			auto directory = std::filesystem::path(test::TempDirectoryGuard::DefaultName()) / name;
			std::filesystem::create_directories(directory);

			// Sanity:
			EXPECT_TRUE(std::filesystem::exists(directory));

			SetIndexes<TTraits>(directory, indexWriterValue, indexReaderValue);
		}

		// endregion
	}

	// region WriterTestContext

	namespace {
		template<typename TTraits>
		class WriterTestContext : public BasicQueueTestContext<TTraits> {
		public:
			WriterTestContext() : WriterTestContext("q")
			{}

			explicit WriterTestContext(const std::filesystem::path& directory)
					: BasicQueueTestContext<TTraits>(directory)
					, m_writer(TTraits::CreateWriter(BasicQueueTestContext<TTraits>::directory().generic_string()))
			{}

		public:
			OutputStream& writer() {
				return m_writer;
			}

		private:
			FileQueueWriter m_writer;
		};
	}

	// endregion

	// region FileQueueWriter - constructor

	DIRECTORY_TRAITS_BASED_TEST(CanCreateQueueWriterAroundNewDirectory) {
		// Act:
		WriterTestContext<TTraits> context;

		// Assert:
		EXPECT_EQ(1u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Writer_Filename));

		EXPECT_EQ(0u, context.readIndexWriterFile());
	}

	DIRECTORY_TRAITS_BASED_TEST(CanCreateQueueWriterAroundExistingDirectory) {
		// Arrange:
		CreateDirectory<TTraits>("qq", 0, 0);

		// Act:
		WriterTestContext<TTraits> context("qq");

		// Assert:
		EXPECT_EQ(1u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Writer_Filename));

		EXPECT_EQ(0u, context.readIndexWriterFile());
	}

	DIRECTORY_TRAITS_BASED_TEST(CanCreateQueueWriterAroundExistingDirectoryWithIndexFile) {
		// Arrange:
		CreateDirectory<TTraits>("qq", 122, 0);

		// Act:
		WriterTestContext<TTraits> context("qq");

		// Assert:
		EXPECT_EQ(1u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Writer_Filename));

		EXPECT_EQ(122u, context.readIndexWriterFile());
	}

	// endregion

	// region FileQueueWriter - write payload(s)

	namespace {
		std::vector<uint8_t> Merge(const std::vector<std::vector<uint8_t>>& buffers) {
			std::vector<uint8_t> mergedBuffer;
			for (const auto& buffer : buffers) {
				auto initialSize = mergedBuffer.size();
				mergedBuffer.resize(initialSize + buffer.size());
				std::memcpy(mergedBuffer.data() + initialSize, buffer.data(), buffer.size());
			}

			return mergedBuffer;
		}
	}

	DIRECTORY_TRAITS_BASED_TEST(CanWriteSinglePayloadToSingleFile) {
		// Arrange:
		WriterTestContext<TTraits> context;
		auto buffer = test::GenerateRandomVector(21);

		// Act:
		context.writer().write(buffer);
		context.writer().flush();

		// Assert:
		EXPECT_EQ(2u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Writer_Filename));
		EXPECT_TRUE(context.exists("0000000000000000.dat"));

		EXPECT_EQ(1u, context.readIndexWriterFile());
		EXPECT_EQ(buffer, context.readAll("0000000000000000.dat"));
	}

	DIRECTORY_TRAITS_BASED_TEST(CanWriteMultiplePayloadsToSingleFile) {
		// Arrange:
		WriterTestContext<TTraits> context;
		std::vector<std::vector<uint8_t>> buffers{
			test::GenerateRandomVector(21),
			test::GenerateRandomVector(80),
			test::GenerateRandomVector(11)
		};

		// Act:
		for (const auto& buffer : buffers)
			context.writer().write(buffer);

		context.writer().flush();

		// Assert:
		EXPECT_EQ(2u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Writer_Filename));
		EXPECT_TRUE(context.exists("0000000000000000.dat"));

		EXPECT_EQ(1u, context.readIndexWriterFile());
		EXPECT_EQ(Merge(buffers), context.readAll("0000000000000000.dat"));
	}

	DIRECTORY_TRAITS_BASED_TEST(CanWriteMultiplePayloadsToMultipleFiles) {
		// Arrange:
		WriterTestContext<TTraits> context;
		std::vector<std::vector<uint8_t>> buffers{
			test::GenerateRandomVector(21),
			test::GenerateRandomVector(80),
			test::GenerateRandomVector(11)
		};

		// Act:
		for (const auto& buffer : buffers) {
			context.writer().write(buffer);
			context.writer().flush();
		}

		// Assert:
		EXPECT_EQ(4u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Writer_Filename));
		EXPECT_TRUE(context.exists("0000000000000000.dat"));
		EXPECT_TRUE(context.exists("0000000000000001.dat"));
		EXPECT_TRUE(context.exists("0000000000000002.dat"));

		EXPECT_EQ(3u, context.readIndexWriterFile());
		EXPECT_EQ(buffers[0], context.readAll("0000000000000000.dat"));
		EXPECT_EQ(buffers[1], context.readAll("0000000000000001.dat"));
		EXPECT_EQ(buffers[2], context.readAll("0000000000000002.dat"));
	}

	// endregion

	// region FileQueueWriter - edge cases

	DIRECTORY_TRAITS_BASED_TEST(WriteBuffersDataInMemory) {
		// Arrange:
		WriterTestContext<TTraits> context;
		auto buffer = test::GenerateRandomVector(21);

		// Act:
		context.writer().write(buffer);

		// Assert:
		EXPECT_EQ(2u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Writer_Filename));
		EXPECT_TRUE(context.exists("0000000000000000.dat"));

		// - even though file is created, index is incremented when flush is called
		EXPECT_EQ(0u, context.readIndexWriterFile());
	}

	DIRECTORY_TRAITS_BASED_TEST(FlushDoesNothingWhenNoPendingDataToWrite) {
		// Arrange:
		WriterTestContext<TTraits> context;
		auto buffer = test::GenerateRandomVector(21);

		// Act:
		context.writer().flush();

		// Assert:
		EXPECT_EQ(1u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Writer_Filename));

		EXPECT_EQ(0u, context.readIndexWriterFile());
	}

	// endregion

	// region ReaderTestContext

	template<typename TTraits>
	class ReaderTestContext : public BasicQueueTestContext<TTraits> {
	public:
		ReaderTestContext() : ReaderTestContext("q")
		{}

		explicit ReaderTestContext(const std::filesystem::path& directory)
				: BasicQueueTestContext<TTraits>(directory)
				, m_reader(TTraits::CreateReader(BasicQueueTestContext<TTraits>::directory().generic_string()))
		{}

	public:
		FileQueueReader& reader() {
			return m_reader;
		}

	public:
		void setIndexes(uint64_t indexWriterValue, uint64_t indexReaderValue) {
			SetIndexes<TTraits>(BasicQueueTestContext<TTraits>::directory(), indexWriterValue, indexReaderValue);
		}

		void write(const std::string& name, const std::vector<uint8_t>& buffer) {
			RawFile dataFile((BasicQueueTestContext<TTraits>::directory() / name).generic_string(), OpenMode::Read_Write);
			dataFile.write(buffer);
		}

	private:
		FileQueueReader m_reader;
	};

	// endregion

	// region FileQueueReader - constructor

	DIRECTORY_TRAITS_BASED_TEST(CanCreateQueueReaderAroundNewDirectory) {
		// Act:
		ReaderTestContext<TTraits> context;

		// Assert:
		EXPECT_EQ(1u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Reader_Filename));

		EXPECT_EQ(0u, context.readIndexReaderFile());
	}

	DIRECTORY_TRAITS_BASED_TEST(CanCreateQueueReaderAroundExistingDirectory) {
		// Arrange:
		CreateDirectory<TTraits>("qq", 0, 0);

		// Act:
		ReaderTestContext<TTraits> context("qq");

		// Assert:
		EXPECT_EQ(1u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Reader_Filename));

		EXPECT_EQ(0u, context.readIndexReaderFile());
	}

	DIRECTORY_TRAITS_BASED_TEST(CanCreateQueueReaderAroundExistingDirectoryWithIndexFile) {
		// Arrange:
		CreateDirectory<TTraits>("qq", 122, 120);

		// Act:
		ReaderTestContext<TTraits> context("qq");

		// Assert:
		EXPECT_EQ(2u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Writer_Filename));
		EXPECT_TRUE(context.exists(TTraits::Index_Reader_Filename));

		EXPECT_EQ(122u, context.readIndexWriterFile());
		EXPECT_EQ(120u, context.readIndexReaderFile());
	}

	// endregion

	// region FileQueueReader - pending

	namespace {
		template<typename TTraits>
		void AssertPendingCount(uint64_t indexWriterValue, uint64_t indexReaderValue, size_t expectedPendingCount) {
			// Arrange:
			ReaderTestContext<TTraits> context;
			context.setIndexes(indexWriterValue, indexReaderValue);

			// Act:
			auto pending = context.reader().pending();

			// Assert:
			EXPECT_EQ(expectedPendingCount, pending) << "W: " << indexWriterValue << ", R: " << indexReaderValue;
		}
	}

	DIRECTORY_TRAITS_BASED_TEST(PendingReturnsCorrectValueWhenWriterIndexDoesNotExist) {
		// Arrange:
		ReaderTestContext<TTraits> context;
		context.setIndexes(121, 120);
		std::filesystem::remove(context.directory() / TTraits::Index_Writer_Filename);

		// Act:
		auto pending = context.reader().pending();

		// Assert:
		EXPECT_EQ(0u, pending);
	}

	DIRECTORY_TRAITS_BASED_TEST(PendingReturnsCorrectValueWhenWriterIndexIsLessThanReaderIndex) {
		AssertPendingCount<TTraits>(119, 120, 0);
		AssertPendingCount<TTraits>(72, 120, 0);
		AssertPendingCount<TTraits>(1, 120, 0);
		AssertPendingCount<TTraits>(0, 120, 0);
	}

	DIRECTORY_TRAITS_BASED_TEST(PendingReturnsCorrectValueWhenWriterIndexIsEqualToReaderIndex) {
		AssertPendingCount<TTraits>(120, 120, 0);
		AssertPendingCount<TTraits>(72, 72, 0);
		AssertPendingCount<TTraits>(1, 1, 0);
		AssertPendingCount<TTraits>(0, 0, 0);
	}

	DIRECTORY_TRAITS_BASED_TEST(PendingReturnsCorrectValueWhenWriterIndexIsGreaterThanReaderIndex) {
		AssertPendingCount<TTraits>(120, 119, 1);
		AssertPendingCount<TTraits>(120, 72, 48);
		AssertPendingCount<TTraits>(120, 1, 119);
		AssertPendingCount<TTraits>(120, 0, 120);
	}

	// endregion

	// region FileQueueReader - read (no data ready)

	namespace {
		[[noreturn]]
		void ReadNever(const std::vector<uint8_t>&) {
			CATAPULT_THROW_INVALID_ARGUMENT("consumer was unexpectedly called");
		}

		template<typename TTraits>
		void AssertIndexFiles(
				const ReaderTestContext<TTraits>& context,
				uint64_t expectedIndexWriterValue,
				uint64_t expectedIndexReaderValue) {
			// Assert:
			EXPECT_TRUE(context.exists(TTraits::Index_Writer_Filename));
			EXPECT_TRUE(context.exists(TTraits::Index_Reader_Filename));

			EXPECT_EQ(expectedIndexWriterValue, context.readIndexWriterFile());
			EXPECT_EQ(expectedIndexReaderValue, context.readIndexReaderFile());
		}

		template<typename TTraits>
		void AssertCannotReadWithIndexValues(uint64_t indexWriterValue, uint64_t indexReaderValue) {
			// Arrange:
			ReaderTestContext<TTraits> context;
			context.setIndexes(indexWriterValue, indexReaderValue);

			// Act:
			auto result = context.reader().tryReadNextMessage(ReadNever);

			// Assert:
			EXPECT_FALSE(result);

			EXPECT_EQ(2u, context.countFiles());
			AssertIndexFiles(context, indexWriterValue, indexReaderValue);
		}
	}

	DIRECTORY_TRAITS_BASED_TEST(CannotReadWhenWriterIndexDoesNotExist) {
		// Arrange:
		ReaderTestContext<TTraits> context;
		context.setIndexes(121, 120);
		std::filesystem::remove(context.directory() / TTraits::Index_Writer_Filename);

		// Act:
		auto result = context.reader().tryReadNextMessage(ReadNever);

		// Assert:
		EXPECT_FALSE(result);

		EXPECT_EQ(1u, context.countFiles());
		EXPECT_TRUE(context.exists(TTraits::Index_Reader_Filename));
		EXPECT_EQ(120u, context.readIndexReaderFile());
	}

	DIRECTORY_TRAITS_BASED_TEST(CannotReadWhenReaderIndexIsGreaterThanWriterIndex) {
		AssertCannotReadWithIndexValues<TTraits>(120, 121);
		AssertCannotReadWithIndexValues<TTraits>(120, 500);
	}

	DIRECTORY_TRAITS_BASED_TEST(CannotReadWhenReaderIndexIsEqualToWriterIndex) {
		AssertCannotReadWithIndexValues<TTraits>(120, 120);
	}

	DIRECTORY_TRAITS_BASED_TEST(CannotReadWhenMessageAtReaderIndexDoesNotExist) {
		// Arrange:
		ReaderTestContext<TTraits> context;
		context.setIndexes(120, 118);

		// Act:
		EXPECT_THROW(context.reader().tryReadNextMessage(ReadNever), catapult_runtime_error);

		// Assert: reader index should not have been incremented because no data was processed
		EXPECT_EQ(2u, context.countFiles());
		AssertIndexFiles(context, 120, 118);
	}

	// endregion

	// region FileQueueReader - read (data ready)

	DIRECTORY_TRAITS_BASED_TEST(CanReadWhenReaderIndexIsLessThanWriterIndex) {
		// Arrange:
		ReaderTestContext<TTraits> context;
		context.setIndexes(120, 118);

		constexpr auto Message_Filename = "0000000000000076.dat"; // 118 == 0x76
		auto writeBuffer = test::GenerateRandomVector(21);
		context.write(Message_Filename, writeBuffer);

		// Sanity:
		EXPECT_TRUE(context.exists(Message_Filename));

		// Act:
		auto numCalls = 0u;
		std::vector<uint8_t> readBuffer;
		auto result = context.reader().tryReadNextMessage([&numCalls, &readBuffer](const auto& buffer) {
			++numCalls;
			readBuffer = buffer;
		});

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(1u, numCalls);
		EXPECT_EQ(writeBuffer, readBuffer);

		// - processed data file should have been deleted
		EXPECT_EQ(2u, context.countFiles());
		AssertIndexFiles(context, 120, 119);
	}

	DIRECTORY_TRAITS_BASED_TEST(CanReadAtMostOneFileWhenReaderIndexIsLessThanWriterIndex) {
		// Arrange:
		ReaderTestContext<TTraits> context;
		context.setIndexes(120, 118);

		constexpr auto Message1_Filename = "0000000000000076.dat"; // 118 == 0x76
		constexpr auto Message2_Filename = "0000000000000077.dat"; // 119 == 0x77
		auto writeBuffer1 = test::GenerateRandomVector(21);
		auto writeBuffer2 = test::GenerateRandomVector(17);
		context.write(Message1_Filename, writeBuffer1);
		context.write(Message2_Filename, writeBuffer2);

		// Sanity:
		EXPECT_TRUE(context.exists(Message1_Filename));
		EXPECT_TRUE(context.exists(Message2_Filename));

		// Act:
		auto numCalls = 0u;
		std::vector<uint8_t> readBuffer;
		auto result = context.reader().tryReadNextMessage([&numCalls, &readBuffer](const auto& buffer) {
			++numCalls;
			readBuffer = buffer;
		});

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(1u, numCalls);
		EXPECT_EQ(writeBuffer1, readBuffer);

		// - processed data file should have been deleted
		EXPECT_EQ(3u, context.countFiles());
		AssertIndexFiles(context, 120, 119);

		EXPECT_TRUE(context.exists(Message2_Filename));
		EXPECT_EQ(writeBuffer2, context.readAll(Message2_Filename));
	}

	DIRECTORY_TRAITS_BASED_TEST(ReadDoesNotRemoveUnsuccessfullyProcessedDataFile) {
		// Arrange:
		ReaderTestContext<TTraits> context;
		context.setIndexes(120, 118);

		constexpr auto Message_Filename = "0000000000000076.dat"; // 118 == 0x76
		auto writeBuffer = test::GenerateRandomVector(21);
		context.write(Message_Filename, writeBuffer);

		// Sanity:
		EXPECT_TRUE(context.exists(Message_Filename));

		// Act: trigger a consumer exception
		EXPECT_THROW(context.reader().tryReadNextMessage(ReadNever), catapult_invalid_argument);

		// Assert: data file should not have been deleted because it was not successfully processed
		EXPECT_EQ(3u, context.countFiles());
		AssertIndexFiles(context, 120, 118);

		EXPECT_TRUE(context.exists(Message_Filename));
		EXPECT_EQ(writeBuffer, context.readAll(Message_Filename));
	}

	// endregion

	// region FileQueueReader - skip

	namespace {
		template<typename TTraits>
		void AssertSkipHasNoEffect(uint64_t indexWriterValue, uint64_t indexReaderValue, uint32_t skipCount) {
			// Arrange:
			ReaderTestContext<TTraits> context;
			context.setIndexes(indexWriterValue, indexReaderValue);

			// Act:
			context.reader().skip(skipCount);

			// Assert:
			EXPECT_EQ(2u, context.countFiles());
			AssertIndexFiles(context, indexWriterValue, indexReaderValue);
		}

		template<typename TTraits>
		void AssertCanSkip(uint64_t indexWriterValue, uint64_t indexReaderValue, uint32_t skipCount, uint64_t expectedIndexReaderValue) {
			// Arrange:
			ReaderTestContext<TTraits> context;
			context.setIndexes(indexWriterValue, indexReaderValue);

			for (auto id = indexReaderValue; id < indexWriterValue; ++id) {
				std::ostringstream out;
				out << utils::HexFormat(id) << ".dat";
				context.write(out.str(), test::GenerateRandomVector(15 + id % 10));
			}

			// Act:
			context.reader().skip(skipCount);

			EXPECT_EQ(2u + indexWriterValue - expectedIndexReaderValue, context.countFiles());
			AssertIndexFiles(context, indexWriterValue, expectedIndexReaderValue);
		}
	}

	DIRECTORY_TRAITS_BASED_TEST(SkipHasNoEffectWhenReaderIndexIsGreaterThanWriterIndex) {
		AssertSkipHasNoEffect<TTraits>(120, 130, 5);
	}

	DIRECTORY_TRAITS_BASED_TEST(SkipHasNoEffectWhenReaderIndexIsEqualToWriterIndex) {
		AssertSkipHasNoEffect<TTraits>(120, 120, 5);
	}

	DIRECTORY_TRAITS_BASED_TEST(SkipWillSkipAtMostCountMessages) {
		AssertCanSkip<TTraits>(120, 110, 6, 116);
	}

	DIRECTORY_TRAITS_BASED_TEST(SkipWillNotSkipPastWriterIndex) {
		AssertCanSkip<TTraits>(120, 118, 6, 120);
	}

	// endregion
}}
