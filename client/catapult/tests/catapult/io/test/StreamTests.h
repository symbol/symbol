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

	public:
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

		// region read tests

	public:
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

	public:
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
}}
