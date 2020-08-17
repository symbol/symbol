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

#pragma once
#include "catapult/exceptions.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Container of tests for io::InputStream and io::OutputStream.
	template<typename TContext>
	class StreamTests {
	public:
		static constexpr size_t Default_Test_Buffer_Size = 1000;

	private:
		using ByteBuffer = std::vector<uint8_t>;

		static constexpr size_t Chunk_Size = Default_Test_Buffer_Size / 9;
		static constexpr size_t Num_Chunks = 15;

	public:
		// region eof tests

		static void AssertEmptyStreamIsInitiallyAtEof() {
			// Arrange:
			TContext context("test.dat");
			{
				auto pOutput = context.outputStream();
				pOutput->flush();
			}

			auto pInput = context.inputStream();

			// Act + Assert:
			EXPECT_TRUE(pInput->eof());
		}

		static void AssertStreamWithDataIsInitiallyNotAtEof() {
			// Arrange:
			TContext context("test.dat");
			{
				auto pOutput = context.outputStream();
				pOutput->write(test::GenerateRandomVector(257));
				pOutput->flush();
			}

			auto pInput = context.inputStream();

			// Act + Assert:
			EXPECT_FALSE(pInput->eof());
		}

		static void AssertStreamWithDataCanAdvanceToEof() {
			// Arrange:
			TContext context("test.dat");
			{
				auto pOutput = context.outputStream();
				pOutput->write(test::GenerateRandomVector(257));
				pOutput->flush();
			}

			auto pInput = context.inputStream();

			// Sanity:
			EXPECT_FALSE(pInput->eof());

			// - advance input stream to eof
			ByteBuffer buffer(257);
			pInput->read(buffer);

			// Act + Assert:
			EXPECT_TRUE(pInput->eof());
		}

		static void AssertReadSmallerThanBufferSizeDoesNotMoveToEof() {
			// Arrange:
			constexpr auto Data_Size = Default_Test_Buffer_Size - 13;
			TContext context("test.dat");
			{
				auto pOutput = context.outputStream();
				pOutput->write(test::GenerateRandomVector(Data_Size));
				pOutput->flush();
			}

			auto pInput = context.inputStream();

			// Sanity:
			EXPECT_FALSE(pInput->eof());

			// Act + Assert:
			ByteBuffer buffer(Chunk_Size);
			constexpr auto Num_Reads = Data_Size / Chunk_Size;
			for (auto i = 0u; i < Num_Reads; ++i) {
				pInput->read(buffer);
				EXPECT_FALSE(pInput->eof()) << " at iteration " << i;
			}

			// - read last chunk and assert
			constexpr auto Leftover_Size = Data_Size - Chunk_Size * Num_Reads;
			buffer.resize(Leftover_Size);
			pInput->read(buffer);
			EXPECT_TRUE(pInput->eof());
		}

		// endregion

		// region read/write policies

		struct SingleWritePolicy {
			template<typename TOutput>
			static void Write(TOutput& output, const ByteBuffer& buffer) {
				output.write(buffer);
			}
		};

		struct ChunkedWritePolicy {
			template<typename TOutput>
			static void Write(TOutput& output, const ByteBuffer& buffer) {
				for (auto i = 0u; i < Num_Chunks; ++i) {
					auto beginIter = buffer.begin() + i * Chunk_Size;
					ByteBuffer chunk(beginIter, beginIter + Chunk_Size);
					output.write(chunk);
				}
			}
		};

		struct SingleReadPolicy {
			template<typename TInput>
			static void Read(TInput& input, ByteBuffer& buffer) {
				input.read(buffer);
			}
		};

		struct ChunkedReadPolicy {
			template<typename TInput>
			static void Read(TInput& input, ByteBuffer& buffer) {
				ByteBuffer chunk(Chunk_Size);
				for (auto i = 0u; i < Num_Chunks; ++i) {
					input.read(chunk);
					std::copy(chunk.begin(), chunk.end(), buffer.begin() + i * Chunk_Size);
				}
			}
		};

		// endregion

		// region read/write (multiple streams) tests

		template<typename TWritePolicy, typename TReadPolicy>
		static void RunRoundtripTest() {
			// Arrange:
			constexpr auto Roundtrip_Buffer_Size = Num_Chunks * Chunk_Size;
			TContext context("test.dat");
			auto expected = test::GenerateRandomVector(Roundtrip_Buffer_Size);

			// Act:
			{
				auto pOutput = context.outputStream();
				TWritePolicy::Write(*pOutput, expected);
				pOutput->flush();
			}

			ByteBuffer buffer(Roundtrip_Buffer_Size);
			{
				auto pInput = context.inputStream();
				TReadPolicy::Read(*pInput, buffer);
			}

			// Assert:
			EXPECT_EQ(expected, buffer);
		}

		static void AssertCannotReadMoreThanWritten() {
			// Arrange:
			TContext context("test.dat");
			auto expected = test::GenerateRandomVector(1234);
			{
				auto pOutput = context.outputStream();
				pOutput->write(expected);
				pOutput->flush();
			}

			ByteBuffer buffer(expected.size() + 1);

			// Act + Assert:
			auto pInput = context.inputStream();
			EXPECT_THROW(pInput->read(buffer), catapult_file_io_error);
		}

		// endregion

		// region position/seek tests

		static void AssertPositionIsInitiallyZero() {
			// Arrange:
			TContext context("test.dat");
			auto pStream = context.outputStream();

			// Act + Assert:
			EXPECT_EQ(0u, pStream->position());
			EXPECT_TRUE(pStream->eof());
		}

		static void AssertCanSeekToArbitraryLocation() {
			// Arrange:
			TContext context("test.dat");
			auto pStream = context.outputStream();
			pStream->write(test::GenerateRandomVector(257));

			// Act:
			pStream->seek(198);

			// Assert:
			EXPECT_EQ(198u, pStream->position());
			EXPECT_FALSE(pStream->eof());
		}

		static void AssertCanSeekToEof() {
			// Arrange:
			TContext context("test.dat");
			auto pStream = context.outputStream();
			pStream->write(test::GenerateRandomVector(257));
			pStream->seek(198);

			// Act:
			pStream->seek(257);

			// Assert:
			EXPECT_EQ(257u, pStream->position());
			EXPECT_TRUE(pStream->eof());
		}

		static void AssertCannotSeekPastEof() {
			// Arrange:
			TContext context("test.dat");
			auto pStream = context.outputStream();
			pStream->write(test::GenerateRandomVector(257));
			pStream->seek(198);

			// Act + Assert:
			EXPECT_THROW(pStream->seek(258), catapult_file_io_error);
		}

		static void AssertWriteAdvancesPosition() {
			// Arrange:
			TContext context("test.dat");
			auto pStream = context.outputStream();

			// Act: write some data
			auto buffer = test::GenerateRandomVector(27);
			pStream->write(buffer);

			// Assert: stream is at EOF
			EXPECT_EQ(27u, pStream->position());
			EXPECT_TRUE(pStream->eof());
		}

		static void AssertReadAdvancesPosition() {
			// Arrange:
			TContext context("test.dat");
			auto pStream = context.outputStream();

			// - write some data and seek back
			auto buffer = test::GenerateRandomVector(27);
			pStream->write(buffer);
			pStream->seek(10);

			// Act: read some data
			std::vector<uint8_t> part(12);
			pStream->read(part);

			// Assert: stream is not at EOF
			EXPECT_EQ(22u, pStream->position());
			EXPECT_FALSE(pStream->eof());
		}

		// endregion

		// region read/write/seek (single stream)

		static void AssertCanReadAfterSeek() {
			// Arrange:
			auto buffer = test::GenerateRandomVector(123);
			TContext context("test.dat");
			auto pStream = context.outputStream();
			pStream->write(buffer);

			// Act: seek and read
			std::vector<uint8_t> readBuffer(50);
			pStream->seek(10);
			pStream->read(readBuffer);

			// Assert:
			EXPECT_EQ(60u, pStream->position());
			EXPECT_FALSE(pStream->eof());

			EXPECT_EQ_MEMORY(buffer.data() + 10, readBuffer.data(), readBuffer.size());
		}

		static void AssertCanWriteAfterSeek() {
			// Arrange:
			auto buffer1 = test::GenerateRandomVector(123);
			TContext context("test.dat");
			auto pStream = context.outputStream();
			pStream->write(buffer1);

			// Act: seek and write
			auto buffer2 = test::GenerateRandomVector(123);
			pStream->seek(30);
			pStream->write(buffer2);

			// Assert:
			EXPECT_EQ(153u, pStream->position());
			EXPECT_TRUE(pStream->eof());

			// - read part of buffer
			std::vector<uint8_t> readBuffer(40);
			pStream->seek(20);
			pStream->read(readBuffer);

			EXPECT_EQ_MEMORY(buffer1.data() + 20, readBuffer.data(), 10);
			EXPECT_EQ_MEMORY(buffer2.data(), readBuffer.data() + 10, 30);
		}

		// endregion
	};

#define MAKE_STREAM_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { \
		test::StreamTests<TRAITS_NAME>::Assert##TEST_NAME(); \
	}

#define MAKE_STREAM_ROUNDTRIP_TEST(TRAITS_NAME, WRITE_POLICY, READ_POLICY) \
	TEST(TEST_CLASS, RunRoundtripTest_##WRITE_POLICY##_##READ_POLICY) { \
		using TestsContainer = test::StreamTests<TRAITS_NAME>; \
		TestsContainer::RunRoundtripTest<TestsContainer::WRITE_POLICY, TestsContainer::READ_POLICY>(); \
	}

/// Adds all stream tests for the specified stream traits (\a TRAITS_NAME).
#define DEFINE_STREAM_TESTS(TRAITS_NAME) \
	MAKE_STREAM_TEST(TRAITS_NAME, EmptyStreamIsInitiallyAtEof) \
	MAKE_STREAM_TEST(TRAITS_NAME, StreamWithDataIsInitiallyNotAtEof) \
	MAKE_STREAM_TEST(TRAITS_NAME, StreamWithDataCanAdvanceToEof) \
	MAKE_STREAM_TEST(TRAITS_NAME, ReadSmallerThanBufferSizeDoesNotMoveToEof) \
	\
	MAKE_STREAM_ROUNDTRIP_TEST(TRAITS_NAME, SingleWritePolicy, SingleReadPolicy) \
	MAKE_STREAM_ROUNDTRIP_TEST(TRAITS_NAME, SingleWritePolicy, ChunkedReadPolicy) \
	MAKE_STREAM_ROUNDTRIP_TEST(TRAITS_NAME, ChunkedWritePolicy, SingleReadPolicy) \
	MAKE_STREAM_ROUNDTRIP_TEST(TRAITS_NAME, ChunkedWritePolicy, ChunkedReadPolicy) \
	\
	MAKE_STREAM_TEST(TRAITS_NAME, CannotReadMoreThanWritten)

/// Adds all seekable stream tests for the specified stream traits (\a TRAITS_NAME).
#define DEFINE_SEEKABLE_STREAM_TESTS(TRAITS_NAME) \
	DEFINE_STREAM_TESTS(TRAITS_NAME) \
	\
	MAKE_STREAM_TEST(TRAITS_NAME, PositionIsInitiallyZero) \
	MAKE_STREAM_TEST(TRAITS_NAME, CanSeekToArbitraryLocation) \
	MAKE_STREAM_TEST(TRAITS_NAME, CanSeekToEof) \
	MAKE_STREAM_TEST(TRAITS_NAME, CannotSeekPastEof) \
	MAKE_STREAM_TEST(TRAITS_NAME, WriteAdvancesPosition) \
	MAKE_STREAM_TEST(TRAITS_NAME, ReadAdvancesPosition) \
	\
	MAKE_STREAM_TEST(TRAITS_NAME, CanReadAfterSeek) \
	MAKE_STREAM_TEST(TRAITS_NAME, CanWriteAfterSeek)
}}
