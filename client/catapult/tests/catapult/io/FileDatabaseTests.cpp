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

#include "catapult/io/FileDatabase.h"
#include "catapult/io/RawFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS FileDatabaseTests

	namespace {
		constexpr auto Batch_Size = 5;

		// region test context

		class TestContext {
		public:
			explicit TestContext(size_t batchSize = Batch_Size)
					: m_database(config::CatapultDirectory(m_tempDir.name()), { batchSize, ".bin" })
			{}

		public:
			auto& database() {
				return m_database;
			}

			size_t countDatabaseFiles() const {
				return test::CountFilesAndDirectories(m_tempDir.name());
			}

			size_t countDatabaseFiles(uint32_t groupId) const {
				std::ostringstream filenameStream;
				filenameStream << std::setfill('0') << std::setw(5) << groupId;

				return test::CountFilesAndDirectories(std::filesystem::path(m_tempDir.name()) / filenameStream.str());
			}

			std::vector<uint8_t> readAll(uint32_t fileId, uint32_t groupId = 0) const {
				std::ostringstream filenameStream;
				filenameStream << std::setfill('0') << std::setw(5) << groupId << "/" << std::setw(5) << fileId << ".bin";
				auto filename = (std::filesystem::path(m_tempDir.name()) / filenameStream.str()).generic_string();

				io::RawFile rawFile(filename, io::OpenMode::Read_Only);

				std::vector<uint8_t> contents(rawFile.size());
				rawFile.read(contents);
				return contents;
			}

		private:
			test::TempDirectoryGuard m_tempDir;
			FileDatabase m_database;
		};

		// endregion

		// region test utils

		std::vector<std::vector<uint8_t>> CreatePayloads(std::initializer_list<size_t> sizes) {
			std::vector<std::vector<uint8_t>> payloads;
			for (auto size : sizes)
				payloads.push_back(test::GenerateRandomVector(size));

			return payloads;
		}

		void WriteAll(FileDatabase& database, size_t startId, const std::vector<std::vector<uint8_t>>& payloads, size_t increment = 1) {
			for (auto i = 0u; i < payloads.size(); ++i) {
				auto pOutputStream = database.outputStream(startId + i * increment);
				pOutputStream->write(payloads[i]);
			}
		}

		std::vector<uint8_t> MakeHeader(const std::vector<uint64_t>& offsets) {
			std::vector<uint8_t> buffer(offsets.size() * sizeof(uint64_t));
			std::memcpy(buffer.data(), offsets.data(), buffer.size());
			return buffer;
		}

		std::vector<uint8_t> Concatenate(const std::vector<std::vector<uint8_t>>& buffers) {
			std::vector<uint8_t> aggregateBuffer;
			for (const auto& buffer : buffers) {
				auto originalSize = aggregateBuffer.size();
				aggregateBuffer.resize(originalSize + buffer.size());
				std::memcpy(&aggregateBuffer[originalSize], &buffer[0], buffer.size());
			}

			return aggregateBuffer;
		}

		// endregion
	}

	// region basic

	TEST(TEST_CLASS, NoFilesAreCreatedByConstructor) {
		// Act:
		TestContext context;

		// Assert:
		EXPECT_EQ(0u, context.countDatabaseFiles());
	}

	TEST(TEST_CLASS, CannotCreateWithZeroBatchSize) {
		EXPECT_THROW(TestContext(0), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, InputStreamDoesNotCreateFile) {
		// Arrange:
		TestContext context;

		// Act + Assert:
		EXPECT_THROW(context.database().inputStream(0), catapult_file_io_error);

		EXPECT_EQ(0u, context.countDatabaseFiles());
	}

	TEST(TEST_CLASS, OutputStreamCreatesFile) {
		// Arrange:
		TestContext context;

		// Act:
		{
			auto pOutputStream = context.database().outputStream(0);
		}

		// Assert:
		EXPECT_EQ(1u, context.countDatabaseFiles());
		EXPECT_EQ(1u, context.countDatabaseFiles(0));

		auto contents = context.readAll(0);
		EXPECT_EQ(MakeHeader({ 40, 0, 0, 0, 0 }), contents);
	}

	// endregion

	// region contains

	namespace {
		void RunContainsTest(uint64_t idToCheck, bool expectedResult) {
			// Arrange:
			TestContext context;

			auto payloads = CreatePayloads({ 50, 10, 30 });
			WriteAll(context.database(), 10, payloads);

			// Act:
			auto contains = context.database().contains(idToCheck);

			// Assert:
			EXPECT_EQ(expectedResult, contains);
		}
	}

	TEST(TEST_CLASS, ContainsReturnsFalseWhenFileDoesNotExist) {
		RunContainsTest(15, false);
	}

	TEST(TEST_CLASS, ContainsReturnsFalseWhenFileExistsButPayloadDoesNot) {
		RunContainsTest(13, false);
	}

	TEST(TEST_CLASS, ContainsReturnsTrueWhenFileAndPayloadExist) {
		RunContainsTest(11, true);
	}

	namespace {
		void RunHeaderlessModeContainsTest(uint64_t idToCheck, bool expectedResult) {
			// Arrange:
			TestContext context(1);

			auto payloads = CreatePayloads({ 50, 1, 30 });
			WriteAll(context.database(), 10, payloads);

			// Act:
			auto contains = context.database().contains(idToCheck);

			// Assert:
			EXPECT_EQ(expectedResult, contains);
		}
	}

	TEST(TEST_CLASS, ContainsReturnsFalseWhenFileDoesNotExist_HeaderlesssMode) {
		RunHeaderlessModeContainsTest(13, false);
	}

	TEST(TEST_CLASS, ContainsReturnsTrueWhenFileExists_HeaderlesssMode) {
		RunHeaderlessModeContainsTest(11, true); // notice that header check is bypassed
	}

	// endregion

	// region write - single file

	namespace {
		void AssertCanWriteSinglePayload(size_t id, uint32_t expectedFileId, const std::vector<uint64_t>& expectedHeaderOffsets) {
			// Arrange:
			TestContext context;

			// Act:
			auto payload = test::GenerateRandomVector(50);
			WriteAll(context.database(), id, { payload });

			// Assert:
			EXPECT_EQ(1u, context.countDatabaseFiles());
			EXPECT_EQ(1u, context.countDatabaseFiles(0));

			auto contents = context.readAll(expectedFileId);
			EXPECT_EQ(Concatenate({ MakeHeader(expectedHeaderOffsets), payload }), contents);
		}
	}

	TEST(TEST_CLASS, CanWriteFirstPayloadInFile) {
		AssertCanWriteSinglePayload(5, 5, { 40, 0, 0, 0, 0 });
	}

	TEST(TEST_CLASS, CanWriteMiddlePayloadInFile) {
		AssertCanWriteSinglePayload(7, 5, { 0, 0, 40, 0, 0 });
	}

	TEST(TEST_CLASS, CanWriteLastPayloadInFile) {
		AssertCanWriteSinglePayload(9, 5, { 0, 0, 0, 0, 40 });
	}

	TEST(TEST_CLASS, CanWriteMultiplePayloadsToSingleFile) {
		// Arrange:
		TestContext context;

		// Act:
		auto payloads = CreatePayloads({ 50, 10 });
		WriteAll(context.database(), 10, payloads);

		// Assert:
		EXPECT_EQ(1u, context.countDatabaseFiles());
		EXPECT_EQ(1u, context.countDatabaseFiles(0));

		auto contents = context.readAll(10);
		EXPECT_EQ(Concatenate({ MakeHeader({ 40, 90, 0, 0, 0 }), payloads[0], payloads[1] }), contents);
	}

	TEST(TEST_CLASS, CanWriteMultiplePayloadsToFillSingleFile) {
		// Arrange:
		TestContext context;

		// Act:
		auto payloads = CreatePayloads({ 50, 10, 30, 20, 15 });
		WriteAll(context.database(), 10, payloads);

		// Assert:
		EXPECT_EQ(1u, context.countDatabaseFiles());
		EXPECT_EQ(1u, context.countDatabaseFiles(0));

		auto contents = context.readAll(10);
		EXPECT_EQ(
				Concatenate({ MakeHeader({ 40, 90, 100, 130, 150 }), payloads[0], payloads[1], payloads[2], payloads[3], payloads[4] }),
				contents);
	}

	// endregion

	// region rewrite - single file

	namespace {
		template<typename TAction>
		void RunRewriteTest(size_t rewriteId, TAction action) {
			// Arrange:
			TestContext context;

			auto payloads = CreatePayloads({ 50, 10, 30, 20, 15 });
			WriteAll(context.database(), 10, payloads);

			// Act:
			auto newPayload = test::GenerateRandomVector(50);
			WriteAll(context.database(), rewriteId, { newPayload });

			// Assert:
			EXPECT_EQ(1u, context.countDatabaseFiles());
			EXPECT_EQ(1u, context.countDatabaseFiles(0));

			auto contents = context.readAll(10);
			action(contents, payloads, newPayload);
		}
	}

	TEST(TEST_CLASS, CanRewriteFirstPayloadInFile) {
		RunRewriteTest(10, [](const auto& contents, const auto&, const auto& newPayload) {
			EXPECT_EQ(Concatenate({ MakeHeader({ 40, 0, 0, 0, 0 }), newPayload }), contents);
		});
	}

	TEST(TEST_CLASS, CanRewriteMiddlePayloadInFile) {
		RunRewriteTest(12, [](const auto& contents, const auto& payloads, const auto& newPayload) {
			EXPECT_EQ(Concatenate({ MakeHeader({ 40, 90, 100, 0, 0 }), payloads[0], payloads[1], newPayload }), contents);
		});
	}

	TEST(TEST_CLASS, CanRewriteLastPayloadInFile) {
		RunRewriteTest(14, [](const auto& contents, const auto& payloads, const auto& newPayload) {
			EXPECT_EQ(
					Concatenate({ MakeHeader({ 40, 90, 100, 130, 150 }), payloads[0], payloads[1], payloads[2], payloads[3], newPayload }),
					contents);
		});
	}

	// endregion

	// region write - gaps

	TEST(TEST_CLASS, CanSkipIdsWhenWritingInSingleFile) {
		// Arrange:
		TestContext context;

		// Act:
		auto payloads = CreatePayloads({ 50, 10, 30 });
		WriteAll(context.database(), 10, payloads, 2);

		// Assert:
		EXPECT_EQ(1u, context.countDatabaseFiles());
		EXPECT_EQ(1u, context.countDatabaseFiles(0));

		auto contents = context.readAll(10);
		EXPECT_EQ(Concatenate({ MakeHeader({ 40, 0, 90, 0, 100 }), payloads[0], payloads[1], payloads[2] }), contents);
	}

	TEST(TEST_CLASS, CanFillSkippedIdsWhenWritingInSingleFile) {
		// Arrange:
		TestContext context;

		auto payloads = CreatePayloads({ 50, 10, 30 });
		WriteAll(context.database(), 10, payloads, 2);

		// Act:
		auto newPayload = test::GenerateRandomVector(50);
		WriteAll(context.database(), 11, { newPayload });

		// Assert:
		EXPECT_EQ(1u, context.countDatabaseFiles());
		EXPECT_EQ(1u, context.countDatabaseFiles(0));

		auto contents = context.readAll(10);
		EXPECT_EQ(Concatenate({ MakeHeader({ 40, 130, 90, 0, 100 }), payloads[0], payloads[1], payloads[2], newPayload }), contents);
	}

	// endregion

	// region write / rewrite - multiple files

	TEST(TEST_CLASS, CanWriteAcrossMultipleFiles) {
		// Arrange:
		TestContext context;

		// Act:
		auto payloads = CreatePayloads({ 50, 10, 30, 10, 20, 90, 40, 60 });
		WriteAll(context.database(), 13, payloads);

		// Assert:
		EXPECT_EQ(1u, context.countDatabaseFiles());
		EXPECT_EQ(3u, context.countDatabaseFiles(0));

		auto contents2 = context.readAll(10);
		auto contents3 = context.readAll(15);
		auto contents4 = context.readAll(20);
		EXPECT_EQ(Concatenate({ MakeHeader({ 0, 0, 0, 40, 90 }), payloads[0], payloads[1] }), contents2);
		EXPECT_EQ(
				Concatenate({ MakeHeader({ 40, 70, 80, 100, 190 }), payloads[2], payloads[3], payloads[4], payloads[5], payloads[6] }),
				contents3);
		EXPECT_EQ(Concatenate({ MakeHeader({ 40, 0, 0, 0, 0 }), payloads[7] }), contents4);
	}

	TEST(TEST_CLASS, CanRewritePayloadInMiddleFile) {
		// Arrange:
		TestContext context;

		auto payloads = CreatePayloads({ 50, 10, 30, 10, 20, 90, 40, 60 });
		WriteAll(context.database(), 13, payloads);

		// Act:
		auto newPayload = test::GenerateRandomVector(50);
		WriteAll(context.database(), 17, { newPayload });

		// Assert: as an optimization, files after the rewritten payload are unmodified
		EXPECT_EQ(1u, context.countDatabaseFiles());
		EXPECT_EQ(3u, context.countDatabaseFiles(0));

		auto contents2 = context.readAll(10);
		auto contents3 = context.readAll(15);
		auto contents4 = context.readAll(20);
		EXPECT_EQ(Concatenate({ MakeHeader({ 0, 0, 0, 40, 90 }), payloads[0], payloads[1] }), contents2);
		EXPECT_EQ(Concatenate({ MakeHeader({ 40, 70, 80, 0, 0 }), payloads[2], payloads[3], newPayload }), contents3);
		EXPECT_EQ(Concatenate({ MakeHeader({ 40, 0, 0, 0, 0 }), payloads[7] }), contents4); // effectively orphaned
	}

	// endregion

	// region read

#define READ_TEST(TEST_NAME) \
	template<size_t Payload_Index> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_FirstPayloadInFile) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<0>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MiddlePayloadInFile) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<2>(); } \
	TEST(TEST_CLASS, TEST_NAME##_LastPayloadInFile) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<4>(); } \
	template<size_t Payload_Index> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	READ_TEST(CanReadFullPayloadInFile) {
		// Arrange:
		TestContext context;

		auto payloads = CreatePayloads({ 50, 10, 30, 20, 15 });
		WriteAll(context.database(), 10, payloads);

		// Act:
		auto pInputStream = context.database().inputStream(10 + Payload_Index);

		std::vector<uint8_t> readBuffer(payloads[Payload_Index].size());
		pInputStream->read(readBuffer);

		// Assert:
		EXPECT_EQ(payloads[Payload_Index], readBuffer);
		EXPECT_TRUE(pInputStream->eof());
	}

	READ_TEST(CanReadPartialPayloadInFile) {
		// Arrange:
		TestContext context;

		auto payloads = CreatePayloads({ 50, 10, 30, 20, 15 });
		WriteAll(context.database(), 10, payloads);

		// Act:
		auto pInputStream = context.database().inputStream(10 + Payload_Index);

		std::vector<uint8_t> readBuffer(payloads[Payload_Index].size() / 2);
		pInputStream->read(readBuffer);

		// Assert:
		EXPECT_EQ_MEMORY(&payloads[Payload_Index][0], &readBuffer[0], readBuffer.size());
		EXPECT_FALSE(pInputStream->eof());
	}

	READ_TEST(CannotReadPastPayloadInFile) {
		// Arrange:
		TestContext context;

		auto payloads = CreatePayloads({ 50, 10, 30, 20, 15 });
		WriteAll(context.database(), 10, payloads);

		// - read all data from stream
		auto pInputStream = context.database().inputStream(10 + Payload_Index);

		std::vector<uint8_t> readBuffer(payloads[Payload_Index].size());
		pInputStream->read(readBuffer);

		// Sanity:
		EXPECT_TRUE(pInputStream->eof());

		// Act + Assert:
		EXPECT_THROW(pInputStream->read(readBuffer), catapult_file_io_error);
	}

	READ_TEST(CanRetrievePayloadSizeViaRead) {
		// Arrange:
		TestContext context;

		auto payloads = CreatePayloads({ 50, 10, 30, 20, 15 });
		WriteAll(context.database(), 10, payloads);

		// Act:
		size_t streamSize = 0;
		auto pInputStream = context.database().inputStream(10 + Payload_Index, &streamSize);

		// Assert:
		EXPECT_EQ(payloads[Payload_Index].size(), streamSize);
	}

	TEST(TEST_CLASS, CanReadLastPayloadInPartiallyFullFile) {
		// Arrange:
		TestContext context;

		auto payloads = CreatePayloads({ 50, 10, 30 });
		WriteAll(context.database(), 10, payloads);

		// Act:
		auto pInputStream = context.database().inputStream(12);

		std::vector<uint8_t> readBuffer(payloads[2].size());
		pInputStream->read(readBuffer);

		// Assert:
		EXPECT_EQ(payloads[2], readBuffer);
		EXPECT_TRUE(pInputStream->eof());
	}

	TEST(TEST_CLASS, CannotReadUnwrittenPayloadInPartiallyFullFile) {
		// Arrange:
		TestContext context;

		auto payloads = CreatePayloads({ 50, 10, 30 });
		WriteAll(context.database(), 10, payloads);

		// Act + Assert:
		EXPECT_THROW(context.database().inputStream(13), catapult_file_io_error);
	}

	TEST(TEST_CLASS, CannotProperlyReadSkipIds) {
		// Arrange:
		TestContext context;

		auto payloads = CreatePayloads({ 50, 10, 30 });
		WriteAll(context.database(), 10, payloads, 2);

		// Act:
		auto pInputStream = context.database().inputStream(10);

		std::vector<uint8_t> readBuffer(90);
		pInputStream->read(readBuffer);

		// Assert:
		EXPECT_EQ(Concatenate({ payloads[0], payloads[1], payloads[2] }), readBuffer);
		EXPECT_TRUE(pInputStream->eof());
	}

	// endregion

	// region read + write across versioned directories

	TEST(TEST_CLASS, CanWriteAcrossMultipleFilesInMultipleVersionedDirectories) {
		// Arrange:
		TestContext context;

		// Act:
		auto payloads = CreatePayloads({ 50, 10, 30, 10, 20, 90, 40, 60 });
		WriteAll(context.database(), 9998, payloads);

		// Assert:
		EXPECT_EQ(2u, context.countDatabaseFiles());
		EXPECT_EQ(1u, context.countDatabaseFiles(0));
		EXPECT_EQ(2u, context.countDatabaseFiles(1));

		auto contents2 = context.readAll(9995);
		auto contents3 = context.readAll(0, 1);
		auto contents4 = context.readAll(5, 1);
		EXPECT_EQ(Concatenate({ MakeHeader({ 0, 0, 0, 40, 90 }), payloads[0], payloads[1] }), contents2);
		EXPECT_EQ(
				Concatenate({ MakeHeader({ 40, 70, 80, 100, 190 }), payloads[2], payloads[3], payloads[4], payloads[5], payloads[6] }),
				contents3);
		EXPECT_EQ(Concatenate({ MakeHeader({ 40, 0, 0, 0, 0 }), payloads[7] }), contents4);
	}

	TEST(TEST_CLASS, CanReadFromMultipleVersionedDirectories) {
		// Arrange:
		TestContext context;

		auto payloads = CreatePayloads({ 50, 10, 30, 10, 20, 90, 40, 60 });
		WriteAll(context.database(), 9998, payloads);

		// Act:
		auto pInputStream1 = context.database().inputStream(9998);

		std::vector<uint8_t> readBuffer1(payloads[0].size());
		pInputStream1->read(readBuffer1);

		auto pInputStream2 = context.database().inputStream(10005);

		std::vector<uint8_t> readBuffer2(payloads[7].size());
		pInputStream2->read(readBuffer2);

		// Assert:
		EXPECT_EQ(payloads[0], readBuffer1);
		EXPECT_EQ(payloads[7], readBuffer2);
	}

	// endregion

	// region read + write without header

	TEST(TEST_CLASS, CanWriteAcrossMultipleFilesInHeaderlessMode) {
		// Arrange:
		TestContext context(1);

		// Act:
		auto payloads = CreatePayloads({ 50, 10, 30 });
		WriteAll(context.database(), 10, payloads);

		// Assert:
		EXPECT_EQ(1u, context.countDatabaseFiles());
		EXPECT_EQ(3u, context.countDatabaseFiles(0));

		auto contents1 = context.readAll(10);
		auto contents2 = context.readAll(11);
		auto contents3 = context.readAll(12);
		EXPECT_EQ(payloads[0], contents1);
		EXPECT_EQ(payloads[1], contents2);
		EXPECT_EQ(payloads[2], contents3);
	}

	TEST(TEST_CLASS, CanRewriteInHeaderlessMode) {
		// Arrange:
		TestContext context(1);

		auto payloads = CreatePayloads({ 50, 10, 30 });
		WriteAll(context.database(), 10, payloads);

		// Act:
		auto newPayload = test::GenerateRandomVector(7);
		WriteAll(context.database(), 11, { newPayload });

		// Assert:
		EXPECT_EQ(1u, context.countDatabaseFiles());
		EXPECT_EQ(3u, context.countDatabaseFiles(0));

		auto contents1 = context.readAll(10);
		auto contents2 = context.readAll(11);
		auto contents3 = context.readAll(12);
		EXPECT_EQ(payloads[0], contents1);
		EXPECT_EQ(newPayload, contents2);
		EXPECT_EQ(payloads[2], contents3);
	}

	TEST(TEST_CLASS, CanReadInHeaderlessMode) {
		// Arrange:
		TestContext context(1);

		auto payloads = CreatePayloads({ 50, 10, 30 });
		WriteAll(context.database(), 10, payloads);

		// Act:
		auto pInputStream1 = context.database().inputStream(10);

		std::vector<uint8_t> readBuffer1(payloads[0].size());
		pInputStream1->read(readBuffer1);

		auto pInputStream2 = context.database().inputStream(12);

		std::vector<uint8_t> readBuffer2(payloads[2].size());
		pInputStream2->read(readBuffer2);

		// Assert:
		EXPECT_EQ(payloads[0], readBuffer1);
		EXPECT_EQ(payloads[2], readBuffer2);
	}

	TEST(TEST_CLASS, CanRetrievePayloadSizeViaReadInHeaderlessMode) {
		// Arrange:
		TestContext context(1);

		auto payloads = CreatePayloads({ 50, 10, 30 });
		WriteAll(context.database(), 10, payloads);

		// Act:
		size_t size1;
		auto pInputStream1 = context.database().inputStream(10, &size1);

		size_t size2;
		auto pInputStream2 = context.database().inputStream(12, &size2);

		// Assert:
		EXPECT_EQ(50u, size1);
		EXPECT_EQ(30u, size2);
	}

	// endregion
}}
