#pragma once
#include "catapult/exceptions.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

	namespace {
		using ByteBuffer = std::vector<uint8_t>;

		constexpr size_t Default_Test_Buffer_Size = 1000u;
		constexpr auto Chunk_Size = Default_Test_Buffer_Size / 9;
		constexpr size_t Num_Chunks = 15u;
		constexpr auto Roundtrip_Buffer_Size = Chunk_Size * Num_Chunks;

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
	}

	template<typename TContext, typename TWritePolicy, typename TReadPolicy>
	void RunRoundtripTest() {
		// Arrange:
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

#define STREAM_ROUNDTRIP_TEST(STREAM_NAME, WRITE_POLICY, READ_POLICY) \
	TEST(STREAM_NAME##Testss, RunRoundtripTest_##WRITE_POLICY##_##READ_POLICY) { \
		RunRoundtripTest<STREAM_NAME##Context, WRITE_POLICY, READ_POLICY>(); \
	}

/// Adds all stream tests for the specified stream (\a STREAM_NAME).
#define DEFINE_STREAM_TESTS(STREAM_NAME) \
	TEST(STREAM_NAME##Tests, CannotReadMoreThanWritten) { \
		CannotReadMoreThanWritten<STREAM_NAME##Context>(); \
	} \
	\
	STREAM_ROUNDTRIP_TEST(STREAM_NAME, SingleWritePolicy, SingleReadPolicy) \
	STREAM_ROUNDTRIP_TEST(STREAM_NAME, SingleWritePolicy, ChunkedReadPolicy) \
	STREAM_ROUNDTRIP_TEST(STREAM_NAME, ChunkedWritePolicy, SingleReadPolicy) \
	STREAM_ROUNDTRIP_TEST(STREAM_NAME, ChunkedWritePolicy, ChunkedReadPolicy)
}}
