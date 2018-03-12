#include "catapult/io/RawFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

using catapult::test::TempFileGuard;

namespace catapult { namespace io {

#define TEST_CLASS RawFileTests

	namespace {
		static const size_t Default_Bytes_Written = 123u;

		auto WriteRandomVectorToFile(const TempFileGuard &guard, size_t size = Default_Bytes_Written) {
			auto inputData = test::GenerateRandomVector(size);
			RawFile file(guard.name(), OpenMode::Read_Write);
			file.write(inputData);
			EXPECT_EQ(inputData.size(), file.size());
			return inputData;
		}

		auto Slice(const std::vector<uint8_t>& input, size_t start, size_t length) {
			using difference_type = std::vector<uint8_t>::difference_type;
			return std::vector<uint8_t>(
					input.begin() + static_cast<difference_type>(start),
					input.begin() + static_cast<difference_type>(start + length));
		}

		struct WriteTraits {
			static constexpr OpenMode Mode = OpenMode::Read_Write;
		};

		struct AppendTraits {
			static constexpr OpenMode Mode = OpenMode::Read_Append;
		};
	}

#define WRITING_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Write) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<WriteTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Append) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AppendTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TEST(TEST_CLASS, OpeningNonExistingFileThrows) {
		// Arrange:
		TempFileGuard guard("abcdefghijklmnopqrstuvwxyz");

		// Act + Assert:
		EXPECT_THROW(RawFile(guard.name(), OpenMode::Read_Only), catapult_file_io_error);
	}

	WRITING_TRAITS_BASED_TEST(CanOpenFileForWriting) {
		// Arrange:
		TempFileGuard guard("test.dat");
		RawFile r(guard.name(), TTraits::Mode);

		// Assert:
		EXPECT_EQ(0ull, r.size());
		EXPECT_EQ(0ull, r.position());
	}

	WRITING_TRAITS_BASED_TEST(WriteAltersSizeAndPosition) {
		// Arrange:
		TempFileGuard guard("test.dat");
		RawFile r(guard.name(), TTraits::Mode);
		auto inputData = test::GenerateRandomVector(Default_Bytes_Written);

		// Act:
		r.write(inputData);

		// Assert:
		EXPECT_EQ(inputData.size(), r.size());
		EXPECT_EQ(inputData.size(), r.position());
	}

	TEST(TEST_CLASS, WriteOnReadOnlyFileThrowsException) {
		// Arrange:
		TempFileGuard guard("test.dat");
		{
			RawFile r(guard.name(), OpenMode::Read_Write);
		}
		RawFile r(guard.name(), OpenMode::Read_Only);
		auto data = test::GenerateRandomVector(Default_Bytes_Written);

		// Act + Assert:
		EXPECT_THROW(r.write(data), catapult_file_io_error);
	}

	TEST(TEST_CLASS, ReadReturnsProperData) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile r(guard.name(), OpenMode::Read_Only);

		// Act:
		auto outputData = std::vector<uint8_t>(Default_Bytes_Written);
		r.read(outputData);

		// Assert:
		EXPECT_EQ(inputData, outputData);
		EXPECT_EQ(inputData.size(), r.size());
		EXPECT_EQ(inputData.size(), r.position());
	}

	// region move-construct

	TEST(TEST_CLASS, MoveConstructedFilePreservesFileProperties) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile original(guard.name(), OpenMode::Read_Only);
		original.seek(Default_Bytes_Written - 10);

		// Act:
		RawFile r(std::move(original));

		// Assert: position and size are correct
		EXPECT_EQ(Default_Bytes_Written - 10, r.position());
		EXPECT_EQ(inputData.size(), r.size());
	}

	TEST(TEST_CLASS, MovedFileDoesNotSupportFileOperations) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile original(guard.name(), OpenMode::Read_Append);
		original.seek(Default_Bytes_Written - 10);

		// Act:
		RawFile r(std::move(original));

		// Assert:
		auto buffer = std::vector<uint8_t>(10);
		EXPECT_THROW(original.read(buffer), catapult::catapult_runtime_error);
		EXPECT_THROW(original.seek(5), catapult::catapult_runtime_error);
		EXPECT_THROW(original.write(buffer), catapult::catapult_runtime_error);

		// - note that properties are note cleared
		EXPECT_EQ(Default_Bytes_Written - 10, original.position());
		EXPECT_EQ(inputData.size(), original.size());
	}

	TEST(TEST_CLASS, MoveConstructedFileSupportsReading) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile original(guard.name(), OpenMode::Read_Only);
		original.seek(Default_Bytes_Written - 10);

		// Act:
		RawFile r(std::move(original));

		auto dataPart = std::vector<uint8_t>(10);
		r.read(dataPart);

		// Assert:
		auto buffer = std::vector<uint8_t>(10);
		std::vector<uint8_t> expectedPart(inputData.begin() + Default_Bytes_Written - 10, inputData.end());
		EXPECT_EQ(expectedPart, dataPart);
		EXPECT_EQ(inputData.size(), r.size());
		EXPECT_EQ(inputData.size(), r.position());
	}

	// endregion

	// region reading & writing

	TEST(TEST_CLASS, OpenForWriteTruncatesTheFile) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);

		// Act:
		{
			RawFile r(guard.name(), OpenMode::Read_Write);
		}

		// Assert:
		RawFile r(guard.name(), OpenMode::Read_Only);
		EXPECT_EQ(0u, r.size());
		EXPECT_EQ(0u, r.position());
	}

	TEST(TEST_CLASS, AppendCanOverwriteDataAtTheBeginning) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		auto partialData = test::GenerateRandomVector(50);
		{
			RawFile file(guard.name(), OpenMode::Read_Append);
			file.write(partialData);
		}
		RawFile r(guard.name(), OpenMode::Read_Only);

		// Act:
		auto outputData = std::vector<uint8_t>(Default_Bytes_Written);
		r.read(outputData);

		// Assert:
		EXPECT_EQ(partialData, Slice(outputData, 0, 50));
		constexpr size_t Untouched_Size = Default_Bytes_Written - 50;
		EXPECT_EQ(Slice(inputData, 50, Untouched_Size), Slice(outputData, 50, Untouched_Size));

		EXPECT_EQ(outputData.size(), r.size());
		EXPECT_EQ(outputData.size(), r.position());
	}

	TEST(TEST_CLASS, AppendCanOverwriteDataInTheMiddle) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		auto partialData = test::GenerateRandomVector(50);
		{
			RawFile file(guard.name(), OpenMode::Read_Append);
			file.seek(50);
			file.write(partialData);
		}
		RawFile r(guard.name(), OpenMode::Read_Only);

		// Act:
		auto outputData = std::vector<uint8_t>(Default_Bytes_Written);
		r.read(outputData);

		// Assert:
		EXPECT_EQ(Slice(inputData, 0, 50), Slice(outputData, 0, 50));
		EXPECT_EQ(partialData, Slice(outputData, 50, 50));
		constexpr size_t Leftover_Size = Default_Bytes_Written - 100;
		EXPECT_EQ(Slice(inputData, 100, Leftover_Size), Slice(outputData, 100, Leftover_Size));

		EXPECT_EQ(outputData.size(), r.size());
		EXPECT_EQ(outputData.size(), r.position());
	}

	TEST(TEST_CLASS, AppendCanOverwriteDataNearTheEnd) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		auto partialData = test::GenerateRandomVector(50);
		{
			RawFile file(guard.name(), OpenMode::Read_Append);
			file.seek(100);
			file.write(partialData);
		}
		RawFile r(guard.name(), OpenMode::Read_Only);

		// Act:
		auto outputData = std::vector<uint8_t>(150);
		r.read(outputData);

		// Assert:
		EXPECT_EQ(Slice(inputData, 0, 100), Slice(outputData, 0, 100));
		EXPECT_EQ(partialData, Slice(outputData, 100, 50));

		EXPECT_EQ(outputData.size(), r.size());
		EXPECT_EQ(outputData.size(), r.position());
	}

	TEST(TEST_CLASS, AppendCanOverwriteDataAtTheEnd) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		auto finalData = test::GenerateRandomVector(50);
		{
			RawFile file(guard.name(), OpenMode::Read_Append);
			file.seek(file.size());
			file.write(finalData);
		}
		RawFile r(guard.name(), OpenMode::Read_Only);

		// Act:
		auto outputData = std::vector<uint8_t>(Default_Bytes_Written + 50);
		r.read(outputData);

		// Assert:
		EXPECT_EQ(inputData, Slice(outputData, 0, Default_Bytes_Written));
		EXPECT_EQ(finalData, Slice(outputData, Default_Bytes_Written, 50));

		EXPECT_EQ(outputData.size(), r.size());
		EXPECT_EQ(outputData.size(), r.position());
	}

	TEST(TEST_CLASS, ReadReturnsConsecutiveChunks) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile r(guard.name(), OpenMode::Read_Only);

		// Act:
		auto outputData = std::vector<uint8_t>(Default_Bytes_Written);
		r.read({ outputData.data(), 100 });
		r.read({ outputData.data() + 100, 23 });

		// Assert:
		EXPECT_EQ(inputData, outputData);
		EXPECT_EQ(inputData.size(), r.size());
		EXPECT_EQ(inputData.size(), r.position());
	}

	// endregion

	// region seeking + OOB seeking

	WRITING_TRAITS_BASED_TEST(OobSeekInWritableFileThrows) {
		// Arrange:
		TempFileGuard guard("test.dat");
		RawFile r(guard.name(), TTraits::Mode);

		// Act + Assert:
		EXPECT_EQ(0ull, r.size());
		EXPECT_THROW(r.seek(10ull), catapult_file_io_error);
	}

	TEST(TEST_CLASS, OobSeekInRoFileThrows) {
		// Arrange:
		TempFileGuard guard("test.dat");
		{
			RawFile r(guard.name(), OpenMode::Read_Write);
		}
		RawFile r(guard.name(), OpenMode::Read_Only);

		// Act + Assert:
		EXPECT_EQ(0ull, r.size());
		EXPECT_THROW(r.seek(10ull), catapult_file_io_error);
	}

	WRITING_TRAITS_BASED_TEST(CanSeekInsideFile) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = test::GenerateRandomVector(Default_Bytes_Written);
		RawFile r(guard.name(), TTraits::Mode);
		r.write(inputData);

		// Act:
		r.seek(10ull);

		// Assert:
		EXPECT_EQ(inputData.size(), r.size());
		EXPECT_EQ(10ull, r.position());
	}

	WRITING_TRAITS_BASED_TEST(CanSeekToEndOfFile) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = test::GenerateRandomVector(Default_Bytes_Written);
		RawFile r(guard.name(), TTraits::Mode);
		r.write(inputData);
		r.seek(10ull);

		// Act:
		r.seek(inputData.size());

		// Assert:
		EXPECT_EQ(inputData.size(), r.size());
		EXPECT_EQ(inputData.size(), r.position());
	}

	WRITING_TRAITS_BASED_TEST(CannotSeekBeyondWrittenData) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = test::GenerateRandomVector(Default_Bytes_Written);
		RawFile r(guard.name(), TTraits::Mode);
		r.write(inputData);

		// Act + Assert:
		EXPECT_THROW(r.seek(inputData.size() + 1), catapult_file_io_error);
	}

	// endregion

	TEST(TEST_CLASS, ReadThrowsOnOobRead) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile r(guard.name(), OpenMode::Read_Only);
		r.seek(100ull);

		// Act + Assert:
		auto outputData1 = std::vector<uint8_t>(23);
		r.read(outputData1);

		EXPECT_EQ(Default_Bytes_Written, r.size());
		EXPECT_EQ(Default_Bytes_Written, r.position());
		EXPECT_THROW(r.read(outputData1), catapult_file_io_error);
	}

	TEST(TEST_CLASS, ReadingDoesNotAlterFileSize) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile r(guard.name(), OpenMode::Read_Only);

		// Act:
		auto preActionSize = r.size();
		auto preActionPosition = r.position();
		auto outputData1 = std::vector<uint8_t>(Default_Bytes_Written);
		r.read(outputData1);

		// Assert:
		EXPECT_EQ(inputData.size(), preActionSize);
		EXPECT_EQ(0ull, preActionPosition);

		EXPECT_EQ(inputData.size(), r.size());
		EXPECT_EQ(inputData.size(), r.position());
	}

	TEST(TEST_CLASS, CanReadSameData) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile r(guard.name(), OpenMode::Read_Only);

		// Act:
		auto outputData1 = std::vector<uint8_t>(Default_Bytes_Written);
		r.read(outputData1);
		r.seek(23ull);
		auto position = r.position();
		auto outputData2 = std::vector<uint8_t>(Default_Bytes_Written - 23u);
		r.read(outputData2);

		// Assert:
		EXPECT_EQ(inputData.size(), r.size());
		EXPECT_EQ(inputData, outputData1);
		EXPECT_EQ(23ull, position);
		EXPECT_TRUE(std::equal(inputData.cbegin() + 23, inputData.cend(), outputData2.cbegin(), outputData2.cend()));
		EXPECT_EQ(inputData.size(), r.position());
	}

	TEST(TEST_CLASS, PositionInDifferentInstancesIsIndependent) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile r1(guard.name(), OpenMode::Read_Only);
		RawFile r2(guard.name(), OpenMode::Read_Only);

		// Act:
		r1.seek(23);
		r2.seek(50);

		// Assert:
		EXPECT_EQ(23ull, r1.position());
		EXPECT_EQ(50ull, r2.position());
	}

	TEST(TEST_CLASS, CanReadFromTheSameFile) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile r1(guard.name(), OpenMode::Read_Only);
		RawFile r2(guard.name(), OpenMode::Read_Only);

		// Act:
		auto outputData1 = std::vector<uint8_t>(Default_Bytes_Written);
		r1.read(outputData1);
		auto outputData2 = std::vector<uint8_t>(Default_Bytes_Written);
		r2.read(outputData2);

		// Assert:
		EXPECT_EQ(inputData.size(), r1.size());
		EXPECT_EQ(inputData.size(), r2.size());
		EXPECT_EQ(inputData, outputData1);
		EXPECT_EQ(inputData, outputData2);
		EXPECT_EQ(inputData.size(), r1.position());
		EXPECT_EQ(inputData.size(), r2.position());
	}

	// region multiple open with locking

	namespace {
		void AssertCannotOpenForIfOpened(OpenMode alreadyOpenedMode, OpenMode newMode) {
			// Arrange:
			TempFileGuard guard("test.dat");
			// create file if needed
			if (OpenMode::Read_Only == alreadyOpenedMode)
				auto inputData = WriteRandomVectorToFile(guard);

			RawFile r1(guard.name(), alreadyOpenedMode);

			// Act + Assert: all subsequent attempts to open the file should throw
			for (auto i = 0u; i < 3u; ++i)
				EXPECT_THROW(RawFile(guard.name(), newMode), catapult_file_io_error) << "attempt " << i;
		}
	}

	WRITING_TRAITS_BASED_TEST(CannotOpenForWriteIfOpenedForRead) {
		AssertCannotOpenForIfOpened(OpenMode::Read_Only, TTraits::Mode);
	}

	WRITING_TRAITS_BASED_TEST(CannotOpenForReadIfOpenedForWrite) {
		AssertCannotOpenForIfOpened(TTraits::Mode, OpenMode::Read_Only);
	}

	WRITING_TRAITS_BASED_TEST(CannotOpenMixedAppendWrite) {
		auto oppositeMode = OpenMode::Read_Write == TTraits::Mode ? OpenMode::Read_Append : OpenMode::Read_Write;
		AssertCannotOpenForIfOpened(TTraits::Mode, oppositeMode);
	}

	WRITING_TRAITS_BASED_TEST(CannotOpenForWriteIfOpenedForWrite) {
		AssertCannotOpenForIfOpened(TTraits::Mode, TTraits::Mode);
	}

	// endregion

	// region multiple open without locking

	namespace {
		enum class SubsequentOpens {
			Close,
			Keep_Open
		};

		void AssertCanOpenForReadIfOpenedNonLocking(OpenMode alreadyOpenedMode, SubsequentOpens subsequentOpens) {
			// Arrange:
			TempFileGuard guard("test.dat");
			// create file if needed
			RawFile r1(guard.name(), alreadyOpenedMode, LockMode::None);

			// Act + Assert: all subsequent attempts to open the file should succeed
			std::vector<std::unique_ptr<RawFile>> openedFiles;
			for (auto i = 0u; i < 3u; ++i) {
				auto pFile = std::make_unique<RawFile>(guard.name(), OpenMode::Read_Only, LockMode::None);
				EXPECT_EQ(r1.size(), pFile->size());

				if (SubsequentOpens::Keep_Open == subsequentOpens)
					openedFiles.push_back(std::move(pFile));
			}
		}
	}

	WRITING_TRAITS_BASED_TEST(CanOpenForReadIfOpenedForWriteNonLocking) {
		AssertCanOpenForReadIfOpenedNonLocking(TTraits::Mode, SubsequentOpens::Close);
	}

	WRITING_TRAITS_BASED_TEST(CanOpenMultipleForReadIfOpenedForWriteNonLocking) {
		AssertCanOpenForReadIfOpenedNonLocking(TTraits::Mode, SubsequentOpens::Keep_Open);
	}

	TEST(TEST_CLASS, CanOpenForWriteIfOpenedForReadNonLocking) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile r1(guard.name(), OpenMode::Read_Only, LockMode::None);

		// Act + Assert: subsequent attempts to open the file should succeed
		for (auto i = 0u; i < 3u; ++i) {
			RawFile file(guard.name(), OpenMode::Read_Write, LockMode::None);
			EXPECT_EQ(0u, file.size());
			// note: original file reports *old* size, but in our environment that should not be a problem
			EXPECT_EQ(inputData.size(), r1.size());
		}
	}

	TEST(TEST_CLASS, CanOpenForAppendIfOpenedForReadNonLocking) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData = WriteRandomVectorToFile(guard);
		RawFile r1(guard.name(), OpenMode::Read_Only, LockMode::None);

		// Act + Assert: subsequent attempts to open the file should succeed
		for (auto i = 0u; i < 3u; ++i) {
			RawFile file(guard.name(), OpenMode::Read_Append, LockMode::None);
			EXPECT_EQ(inputData.size(), file.size());
			EXPECT_EQ(inputData.size(), r1.size());
		}
	}

	namespace {
		void AssertCanOpenMultipleWhenNonLocking(OpenMode mode1, OpenMode mode2) {
			// Arrange:
			TempFileGuard guard("test.dat");
			auto inputData = WriteRandomVectorToFile(guard);
			RawFile ro0(guard.name(), OpenMode::Read_Only, LockMode::None);

			// Act: all attempts to open should succeed
			RawFile rw1(guard.name(), mode1, LockMode::None);
			RawFile ro1(guard.name(), OpenMode::Read_Only, LockMode::None);

			// Assert:
			size_t expectedSize1 = OpenMode::Read_Write == mode1 ? 0u : inputData.size();
			EXPECT_EQ(expectedSize1, rw1.size());
			EXPECT_EQ(expectedSize1, ro1.size());

			// Act:
			RawFile rw2(guard.name(), mode2, LockMode::None);
			RawFile ro2(guard.name(), OpenMode::Read_Only, LockMode::None);

			// Assert:
			size_t expectedSize2 = (OpenMode::Read_Write == mode1 || OpenMode::Read_Write == mode2) ? 0u : inputData.size();
			EXPECT_EQ(expectedSize2, rw2.size());
			EXPECT_EQ(expectedSize2, ro2.size());
		}
	}

	WRITING_TRAITS_BASED_TEST(CanOpenMultipleWhenNonLocking_SameMode) {
		AssertCanOpenMultipleWhenNonLocking(TTraits::Mode, TTraits::Mode);
	}

	WRITING_TRAITS_BASED_TEST(CanOpenMultipleWhenNonLocking_MixedMode) {
		auto oppositeMode = OpenMode::Read_Write == TTraits::Mode ? OpenMode::Read_Append : OpenMode::Read_Write;
		AssertCanOpenMultipleWhenNonLocking(TTraits::Mode, oppositeMode);
	}

	// endregion

	TEST(TEST_CLASS, ReadAfterOverlappingWriteReturnsProperData) {
		// Arrange:
		TempFileGuard guard("test.dat");
		auto inputData1 = WriteRandomVectorToFile(guard);

		auto outputData1 = std::vector<uint8_t>(Default_Bytes_Written);
		{
			RawFile r(guard.name(), OpenMode::Read_Only);
			r.read(outputData1);
		}

		// Act:
		auto inputData2 = WriteRandomVectorToFile(guard, Default_Bytes_Written + 10);
		RawFile r(guard.name(), OpenMode::Read_Only);
		auto outputData2 = std::vector<uint8_t>(Default_Bytes_Written + 10);
		r.read(outputData2);

		// Assert:
		EXPECT_EQ(inputData2.size(), r.size());
		EXPECT_EQ(inputData1, outputData1);
		EXPECT_EQ(inputData2, outputData2);
		EXPECT_EQ(inputData2.size(), r.position());

	}
}}
