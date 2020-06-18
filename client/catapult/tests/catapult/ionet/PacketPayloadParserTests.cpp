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

#include "catapult/ionet/PacketPayloadParser.h"
#include "catapult/ionet/IoTypes.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketPayloadParserTests

	// region ExtractEntityOffsets / ContainsSingleEntity

	namespace {
		constexpr uint32_t Transaction_Size = sizeof(mocks::MockTransaction);
		constexpr uint32_t Block_Transaction_Size = sizeof(model::BlockHeader) + Transaction_Size;

		void SetTransactionAt(ByteBuffer& buffer, size_t offset) {
			test::SetTransactionAt(buffer, offset, Transaction_Size);
		}

		struct ContainsSingleEntityTraits {
			template<typename TIsValidPredicate>
			static auto Extract(const RawBuffer& buffer, TIsValidPredicate isValid) {
				return ContainsSingleEntity<model::Block>(buffer, isValid);
			}

			static auto Extract(const RawBuffer& buffer) {
				return Extract(buffer, test::DefaultSizeCheck<model::Block>);
			}

			static bool IsEmpty(bool isValid) {
				return !isValid;
			}

			static bool IsSingleEntityResult(bool isValid) {
				return isValid;
			}

			static void PrepareBufferWithOverflowSize(ByteBuffer& buffer, uint32_t size) {
				// create a buffer with no complete blocks but some overflow bytes
				buffer.resize(std::max<uint32_t>(sizeof(model::BlockHeader), size));
				test::SetBlockAt(buffer, 0, buffer.size());
				buffer.resize(size);
			}
		};

		struct ExtractEntityOffsetsTraits {
			template<typename TIsValidPredicate>
			static auto Extract(const RawBuffer& buffer, TIsValidPredicate isValid) {
				return ExtractEntityOffsets<model::Block>(buffer, isValid);
			}

			static auto Extract(const RawBuffer& buffer) {
				return Extract(buffer, test::DefaultSizeCheck<model::Block>);
			}

			static bool IsEmpty(const std::vector<size_t>& offsets) {
				return offsets.empty();
			}

			static bool IsSingleEntityResult(const std::vector<size_t>& offsets) {
				return 1 == offsets.size() && 0 == offsets[0];
			}
		};

		struct ExtractEntityOffsetsSingleEntityTraits : ExtractEntityOffsetsTraits {
			static void PrepareBufferWithOverflowSize(ByteBuffer& buffer, uint32_t size) {
				ContainsSingleEntityTraits::PrepareBufferWithOverflowSize(buffer, size);
			}
		};

		struct ExtractEntityOffsetsMultiEntityTraits : ExtractEntityOffsetsTraits {
			static void PrepareBufferWithOverflowSize(ByteBuffer& buffer, uint32_t size) {
				// create a buffer with two complete blocks and some overflow bytes
				constexpr auto Num_Full_Blocks = 2u;
				constexpr auto Base_Buffer_Size = Num_Full_Blocks * sizeof(model::BlockHeader);
				buffer.resize(Base_Buffer_Size + std::max<uint32_t>(sizeof(model::BlockHeader), size));
				test::SetBlockAt(buffer, 0);
				for (auto i = 0u; i <= Num_Full_Blocks; ++i) {
					auto blockSize = Num_Full_Blocks == i ? size : sizeof(model::BlockHeader);
					test::SetBlockAt(buffer, i * sizeof(model::BlockHeader), blockSize);
				}
			}
		};

		struct CountFixedSizeStructuresTraits {
			static auto Extract(const RawBuffer& buffer) {
				return CountFixedSizeStructures<Hash256>(buffer);
			}

			static bool IsEmpty(size_t numStructures) {
				return 0 == numStructures;
			}
		};

		template<typename TTraits>
		void AssertCannotParseBufferWithSize(uint32_t size) {
			// Arrange:
			ByteBuffer buffer;
			TTraits::PrepareBufferWithOverflowSize(buffer, size);

			// Act:
			auto extractResult = TTraits::Extract(buffer);

			// Assert:
			EXPECT_TRUE(TTraits::IsEmpty(extractResult)) << "overflow size " << size;
		}
	}

#define MAKE_TEST(TEST_NAME, BASE_TRAITS, TEST_NAME_POSTFIX, TRAITS_PREFIX) \
	TEST(TEST_CLASS, TEST_NAME##_##BASE_TRAITS##TEST_NAME_POSTFIX) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BASE_TRAITS##TRAITS_PREFIX##Traits>(); \
	}

#define BUFFER_SIZE_FAILURE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	MAKE_TEST(TEST_NAME, ContainsSingleEntity, ,) \
	MAKE_TEST(TEST_NAME, ExtractEntityOffsets, ,) \
	MAKE_TEST(TEST_NAME, CountFixedSizeStructures, ,) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define BUFFER_FAILURE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	MAKE_TEST(TEST_NAME, ContainsSingleEntity, ,) \
	MAKE_TEST(TEST_NAME, ExtractEntityOffsets, _Single, SingleEntity) \
	MAKE_TEST(TEST_NAME, ExtractEntityOffsets, _Last, MultiEntity) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define BUFFER_SINGLE_ENTITY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	MAKE_TEST(TEST_NAME, ContainsSingleEntity, ,) \
	MAKE_TEST(TEST_NAME, ExtractEntityOffsets, ,) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	BUFFER_SIZE_FAILURE_TEST(CannotExtractFromEmptyBuffer) {
		// Act:
		auto extractResult = TTraits::Extract({ nullptr, 0 });

		// Assert:
		EXPECT_TRUE(TTraits::IsEmpty(extractResult));
	}

	BUFFER_FAILURE_TEST(CannotExtractFromBufferWithoutFullEntityHeader) {
		for (auto size : std::vector<uint32_t>{ 1, sizeof(model::VerifiableEntity) - 1 })
			AssertCannotParseBufferWithSize<TTraits>(size);
	}

	BUFFER_FAILURE_TEST(CannotExtractFromBufferWithoutFullEntityData) {
		for (auto size : std::vector<uint32_t>{ sizeof(model::VerifiableEntity), sizeof(model::BlockHeader) - 1 })
			AssertCannotParseBufferWithSize<TTraits>(size);
	}

	BUFFER_FAILURE_TEST(CannotExtractFromBufferWhenLastEntityExpandsBeyondBuffer) {
		// Arrange:
		// - create a buffer wrapping a block with a transaction
		// - decrease the buffer size by the size of the transaction so that the last entity expands beyond the buffer
		ByteBuffer buffer;
		TTraits::PrepareBufferWithOverflowSize(buffer, Block_Transaction_Size);
		SetTransactionAt(buffer, static_cast<uint32_t>(buffer.size() - Transaction_Size));

		// Act:
		auto extractResult = TTraits::Extract({ buffer.data(), buffer.size() - Transaction_Size });

		// Assert:
		EXPECT_TRUE(TTraits::IsEmpty(extractResult));
	}

	BUFFER_FAILURE_TEST(CannotExtractFromBufferWhenBufferExpandsBeyondLastEntity) {
		// Arrange:
		// - create a buffer wrapping a block
		// - expand the buffer by the size of a transaction so it looks like the buffer expands beyond the last entity
		ByteBuffer buffer;
		TTraits::PrepareBufferWithOverflowSize(buffer, sizeof(model::BlockHeader));
		buffer.resize(buffer.size() + Transaction_Size);

		// Act:
		auto extractResult = TTraits::Extract({ buffer.data(), buffer.size() + Transaction_Size });

		// Assert:
		EXPECT_TRUE(TTraits::IsEmpty(extractResult));
	}

	BUFFER_SINGLE_ENTITY_TEST(CanExtractSingleBlockWithoutTransactions) {
		// Arrange: create a buffer containing a block with no transactions
		ByteBuffer buffer(sizeof(model::BlockHeader));
		test::SetBlockAt(buffer, 0);

		// Act:
		auto extractResult = TTraits::Extract(buffer);

		// Assert:
		EXPECT_TRUE(TTraits::IsSingleEntityResult(extractResult));
	}

	BUFFER_SINGLE_ENTITY_TEST(IsValidPredicateHasHigherPrecedenceThanSizeCheck) {
		// Arrange: create a buffer containing a block with no transactions
		ByteBuffer buffer(sizeof(model::BlockHeader));
		test::SetBlockAt(buffer, 0);

		// Act: extract and return false from the isValid predicate even though the buffer has a valid size
		auto numValidCalls = 0u;
		auto extractResult = TTraits::Extract(buffer, [&numValidCalls](const auto&) {
			++numValidCalls;
			return false;
		});

		// Assert: valid was called once and extraction failed
		EXPECT_EQ(1u, numValidCalls);
		EXPECT_TRUE(TTraits::IsEmpty(extractResult));
	}

	BUFFER_SINGLE_ENTITY_TEST(CanExtractSingleBlockWithTransaction) {
		// Arrange: create a buffer containing a block with one transaction
		ByteBuffer buffer(Block_Transaction_Size);
		test::SetBlockAt(buffer, 0, Block_Transaction_Size);
		SetTransactionAt(buffer, static_cast<uint32_t>(buffer.size() - Transaction_Size));

		// Act:
		auto extractResult = TTraits::Extract(buffer);

		// Assert:
		EXPECT_TRUE(TTraits::IsSingleEntityResult(extractResult));
	}

	namespace {
		void PrepareMultiBlockBuffer(ByteBuffer& buffer) {
			// create a buffer containing three blocks
			buffer.resize(Block_Transaction_Size + 2 * sizeof(model::BlockHeader));
			test::SetBlockAt(buffer, 0); // block 1
			test::SetBlockAt(buffer, sizeof(model::BlockHeader), Block_Transaction_Size); // block 2
			SetTransactionAt(buffer, 2 * sizeof(model::BlockHeader)); // block 2 tx
			test::SetBlockAt(buffer, sizeof(model::BlockHeader) + Block_Transaction_Size); // block 3
		}
	}

	TEST(TEST_CLASS, CanExtractMultipleBlocks_ExtractEntityOffsets) {
		// Arrange: create a buffer containing three blocks
		ByteBuffer buffer;
		PrepareMultiBlockBuffer(buffer);

		// Act:
		auto offsets = ExtractEntityOffsets<model::Block>(buffer, test::DefaultSizeCheck<model::Block>);

		// Assert:
		ASSERT_EQ(3u, offsets.size());
		EXPECT_EQ(std::vector<size_t>({ 0, sizeof(model::BlockHeader), sizeof(model::BlockHeader) + Block_Transaction_Size }), offsets);
	}

	TEST(TEST_CLASS, CannotExtractMultipleBlocks_ContainsSingleEntity) {
		// Arrange: create a buffer containing three blocks
		ByteBuffer buffer;
		PrepareMultiBlockBuffer(buffer);

		// Act:
		auto isValid = ContainsSingleEntity<model::Block>(buffer, test::DefaultSizeCheck<model::Block>);

		// Assert:
		EXPECT_FALSE(isValid);
	}

	// endregion

	// region CountFixedSizeStructures

	namespace {
		constexpr auto Fixed_Size = 62;
		using FixedSizeStructure = std::array<uint8_t, Fixed_Size>;

		void AssertCannotCountFixedSizeStructuresFromBufferWithSize(uint32_t size) {
			// Arrange:
			auto buffer = test::GenerateRandomVector(size);

			// Act:
			auto numStructures = CountFixedSizeStructures<FixedSizeStructure>({ buffer.data(), size });

			// Assert:
			EXPECT_EQ(0u, numStructures) << "buffer size " << size;
		}
	}

	TEST(TEST_CLASS, CannotExtractFromBufferWithPartialStructures_CountFixedSizeStructures) {
		AssertCannotCountFixedSizeStructuresFromBufferWithSize(1);
		AssertCannotCountFixedSizeStructuresFromBufferWithSize(Fixed_Size - 1);
		AssertCannotCountFixedSizeStructuresFromBufferWithSize(Fixed_Size + 1);
		AssertCannotCountFixedSizeStructuresFromBufferWithSize(3 * Fixed_Size - 1);
		AssertCannotCountFixedSizeStructuresFromBufferWithSize(3 * Fixed_Size + 1);
	}

	TEST(TEST_CLASS, CanExtractSingleStructure_CountFixedSizeStructures) {
		// Arrange: create a buffer containing a single fixed size structure
		auto buffer = test::GenerateRandomVector(Fixed_Size);

		// Act:
		auto numStructures = CountFixedSizeStructures<FixedSizeStructure>(buffer);

		// Assert:
		EXPECT_EQ(1u, numStructures);
	}

	TEST(TEST_CLASS, CanExtractMultipleStructures_CountFixedSizeStructures) {
		// Arrange: create a buffer containing three fixed size structures
		auto buffer = test::GenerateRandomVector(3 * Fixed_Size);

		// Act:
		auto numStructures = CountFixedSizeStructures<FixedSizeStructure>(buffer);

		// Assert:
		EXPECT_EQ(3u, numStructures);
	}

	// endregion
}}
