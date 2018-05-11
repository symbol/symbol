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

	using ByteBuffer = std::vector<uint8_t>;

	constexpr size_t Default_Test_Buffer_Size = 1000u;
	constexpr auto Chunk_Size = Default_Test_Buffer_Size / 9;
	constexpr size_t Num_Chunks = 15u;

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
		static void Read(TInput& input, ByteBuffer& result) {
			input.read(result);
		}
	};

	struct ChunkedReadPolicy {
		template<typename TInput>
		static void Read(TInput& input, ByteBuffer& result) {
			ByteBuffer chunk(Chunk_Size);
			for (auto i = 0u; i < Num_Chunks; ++i) {
				input.read(chunk);
				std::copy(chunk.begin(), chunk.end(), result.begin() + i * Chunk_Size);
			}
		}
	};

	template<typename TContext, typename TWritePolicy, typename TReadPolicy>
	void RunRoundtripTest() {
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
		ByteBuffer result(Roundtrip_Buffer_Size);
		{
			auto pInput = context.inputStream();
			TReadPolicy::Read(*pInput, result);
		}

		// Assert:
		EXPECT_EQ(expected, result);
	}

	template<typename TContext>
	void CannotReadMoreThanWritten() {
		// Arrange:
		TContext context("test.dat");
		auto expected = test::GenerateRandomVector(12345);
		{
			auto pOutput = context.outputStream();
			pOutput->write(expected);
			pOutput->flush();
		}
		ByteBuffer result(expected.size() + 1);

		// Act + Assert:
		auto pInput = context.inputStream();
		EXPECT_THROW(pInput->read(result), catapult_file_io_error);
	}

#define MAKE_STREAM_ROUNDTRIP_TEST(TRAITS_NAME, WRITE_POLICY, READ_POLICY) \
	TEST(TEST_CLASS, RunRoundtripTest_##WRITE_POLICY##_##READ_POLICY) { \
		test::RunRoundtripTest<TRAITS_NAME, test::WRITE_POLICY, test::READ_POLICY>(); \
	}

/// Adds all stream tests for the specified stream traits (\a TRAITS_NAME).
#define DEFINE_STREAM_TESTS(TRAITS_NAME) \
	TEST(TEST_CLASS, CannotReadMoreThanWritten) { \
		test::CannotReadMoreThanWritten<TRAITS_NAME>(); \
	} \
	\
	MAKE_STREAM_ROUNDTRIP_TEST(TRAITS_NAME, SingleWritePolicy, SingleReadPolicy) \
	MAKE_STREAM_ROUNDTRIP_TEST(TRAITS_NAME, SingleWritePolicy, ChunkedReadPolicy) \
	MAKE_STREAM_ROUNDTRIP_TEST(TRAITS_NAME, ChunkedWritePolicy, SingleReadPolicy) \
	MAKE_STREAM_ROUNDTRIP_TEST(TRAITS_NAME, ChunkedWritePolicy, ChunkedReadPolicy)
}}
