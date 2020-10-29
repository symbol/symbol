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

#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/IoTypes.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketEntityUtilsTests

	// region CalculatePacketDataSize

	TEST(TEST_CLASS, CalculatePacketDataSizeReturnsZeroWhenPacketIsTooSmall) {
		// Arrange:
		for (auto size : std::initializer_list<uint32_t>{ 0, sizeof(PacketHeader) - 1 }) {
			Packet packet;
			packet.Size = size;

			// Act + Assert:
			EXPECT_EQ(0u, CalculatePacketDataSize(packet));
		}
	}

	TEST(TEST_CLASS, CalculatePacketDataSizeReturnsZeroWhenPacketIsHeaderOnly) {
		// Arrange:
		Packet packet;
		packet.Size = sizeof(PacketHeader);

		// Act + Assert:
		EXPECT_EQ(0u, CalculatePacketDataSize(packet));
	}

	TEST(TEST_CLASS, CalculatePacketDataSizeReturnsDataSizeWhenPacketContainsData) {
		// Arrange:
		for (auto dataSize : std::initializer_list<uint32_t>{ 1, 100 }) {
			Packet packet;
			packet.Size = SizeOf32<PacketHeader>() + dataSize;

			// Act + Assert:
			EXPECT_EQ(dataSize, CalculatePacketDataSize(packet));
		}
	}

	// endregion

	// region ExtractEntitiesFromPacket / ExtractEntityFromPacket

	namespace {
		constexpr auto Default_Packet_Type = PacketType::Push_Block; // packet type is not validated by the parser
		constexpr uint32_t Transaction_Size = sizeof(mocks::MockTransaction);
		constexpr uint32_t Block_Packet_Size = sizeof(Packet) + sizeof(model::BlockHeader);
		constexpr uint32_t Block_Transaction_Size = sizeof(model::BlockHeader) + Transaction_Size;
		constexpr uint32_t Block_Transaction_Packet_Size = sizeof(Packet) + Block_Transaction_Size;

		void SetTransactionAt(ByteBuffer& buffer, size_t offset) {
			test::SetTransactionAt(buffer, offset, Transaction_Size);
		}

		struct ExtractEntityTraits {
			template<typename TIsValidPredicate>
			static auto Extract(const Packet& packet, TIsValidPredicate isValid) {
				return ExtractEntityFromPacket<model::Block>(packet, isValid);
			}

			static auto Extract(const Packet& packet) {
				return Extract(packet, test::DefaultSizeCheck<model::Block>);
			}

			static bool IsEmpty(const std::unique_ptr<model::Block>& pBlock) {
				return !pBlock;
			}

			static void Unwrap(const std::unique_ptr<model::Block>& pBlock, const model::Block*& pBlockOut) {
				ASSERT_TRUE(!!pBlock);
				pBlockOut = pBlock.get();
			}

			static Packet& CreatePacketWithOverflowSize(ByteBuffer& buffer, uint32_t size) {
				// create a packet with no complete blocks but some overflow bytes
				constexpr auto Base_Packet_Size = SizeOf32<Packet>();
				buffer.resize(Base_Packet_Size + std::max<uint32_t>(sizeof(model::BlockHeader), size));
				auto& packet = test::SetPushBlockPacketInBuffer(buffer);
				packet.Size = Base_Packet_Size + size;
				return packet;
			}
		};

		struct ExtractEntitiesTraits {
			template<typename TIsValidPredicate>
			static auto Extract(const Packet& packet, TIsValidPredicate isValid) {
				return ExtractEntitiesFromPacket<model::Block>(packet, isValid);
			}

			static auto Extract(const Packet& packet) {
				return Extract(packet, test::DefaultSizeCheck<model::Block>);
			}

			static bool IsEmpty(const model::BlockRange& range) {
				return range.empty();
			}

			static void Unwrap(const model::BlockRange& range, const model::Block*& pBlockOut) {
				ASSERT_EQ(1u, range.size());
				pBlockOut = range.data();
			}
		};

		struct ExtractEntitiesSingleEntityTraits : ExtractEntitiesTraits {
			static Packet& CreatePacketWithOverflowSize(ByteBuffer& buffer, uint32_t size) {
				return ExtractEntityTraits::CreatePacketWithOverflowSize(buffer, size);
			}
		};

		struct ExtractEntitiesMultiEntityTraits : ExtractEntitiesTraits {
			static Packet& CreatePacketWithOverflowSize(ByteBuffer& buffer, uint32_t size) {
				// create a packet with two complete blocks and some overflow bytes
				constexpr auto Num_Full_Blocks = 2u;
				constexpr auto Base_Packet_Size = SizeOf32<Packet>() + Num_Full_Blocks * SizeOf32<model::BlockHeader>();
				buffer.resize(Base_Packet_Size + std::max<uint32_t>(sizeof(model::BlockHeader), size));
				auto& packet = test::SetPushBlockPacketInBuffer(buffer);
				for (auto i = 0u; i <= Num_Full_Blocks; ++i) {
					auto blockSize = Num_Full_Blocks == i ? size : sizeof(model::BlockHeader);
					test::SetBlockAt(buffer, sizeof(Packet) + i * sizeof(model::BlockHeader), blockSize);
				}

				packet.Size = Base_Packet_Size + size;
				return packet;
			}
		};

		struct ExtractFixedSizeStructuresTraits {
			static auto Extract(const Packet& packet) {
				return ExtractFixedSizeStructuresFromPacket<Hash256>(packet);
			}

			static bool IsEmpty(const model::HashRange& range) {
				return range.empty();
			}
		};

		template<typename TTraits>
		void AssertCannotExtractFromPacketHeaderWithNoData(uint32_t size) {
			// Arrange:
			Packet packet;
			packet.Size = size;
			packet.Type = Default_Packet_Type;

			// Act:
			auto extractResult = TTraits::Extract(packet);

			// Assert:
			EXPECT_TRUE(TTraits::IsEmpty(extractResult)) << "packet size " << size;
		}

		template<typename TTraits>
		void AssertCannotExtractEntitiesFromPacketWithSize(uint32_t size) {
			// Arrange:
			ByteBuffer buffer;
			const auto& packet = TTraits::CreatePacketWithOverflowSize(buffer, size);

			// Act:
			auto extractResult = TTraits::Extract(packet);

			// Assert:
			EXPECT_TRUE(TTraits::IsEmpty(extractResult)) << "overflow size " << size;
		}
	}

#define PACKET_HEADER_FAILURE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntities) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntitiesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_FixedSizeStructures) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractFixedSizeStructuresTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define PACKET_FAILURE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntities_Single) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntitiesSingleEntityTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntities_Last) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntitiesMultiEntityTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define PACKET_SINGLE_ENTITY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntities) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntitiesTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	PACKET_HEADER_FAILURE_TEST(CannotExtractFromPacketWithInvalidSize) {
		AssertCannotExtractFromPacketHeaderWithNoData<TTraits>(sizeof(Packet) - 1);
	}

	PACKET_HEADER_FAILURE_TEST(CannotExtractFromPacketWithNoData) {
		AssertCannotExtractFromPacketHeaderWithNoData<TTraits>(sizeof(Packet));
	}

	PACKET_FAILURE_TEST(CannotExtractFromPacketWithoutFullEntityHeader) {
		for (auto size : std::vector<uint32_t>{ 1, sizeof(model::VerifiableEntity) - 1 })
			AssertCannotExtractEntitiesFromPacketWithSize<TTraits>(size);
	}

	PACKET_FAILURE_TEST(CannotExtractFromPacketWithoutFullEntityData) {
		for (auto size : std::vector<uint32_t>{ sizeof(model::VerifiableEntity), sizeof(model::BlockHeader) - 1 })
			AssertCannotExtractEntitiesFromPacketWithSize<TTraits>(size);
	}

	PACKET_FAILURE_TEST(CannotExtractFromPacketWhenLastEntityExpandsBeyondPacket) {
		// Arrange:
		// - create a packet wrapping a block with a transaction
		// - decrease the packet size by the size of the transaction so that the last entity expands beyond the packet
		ByteBuffer buffer;
		auto& packet = TTraits::CreatePacketWithOverflowSize(buffer, Block_Transaction_Size);
		SetTransactionAt(buffer, static_cast<uint32_t>(buffer.size() - Transaction_Size));
		packet.Size -= Transaction_Size;

		// Act:
		auto extractResult = TTraits::Extract(packet);

		// Assert:
		EXPECT_TRUE(TTraits::IsEmpty(extractResult));
	}

	PACKET_FAILURE_TEST(CannotExtractFromPacketWhenPacketExpandsBeyondLastEntity) {
		// Arrange:
		// - create a packet wrapping a block
		// - expand the buffer by the size of a transaction so it looks like the packet expands beyond the last entity
		ByteBuffer buffer;
		TTraits::CreatePacketWithOverflowSize(buffer, sizeof(model::BlockHeader));
		buffer.resize(buffer.size() + Transaction_Size);
		auto& packet = reinterpret_cast<Packet&>(buffer[0]);
		packet.Size += Transaction_Size;

		// Act:
		auto extractResult = TTraits::Extract(packet);

		// Assert:
		EXPECT_TRUE(TTraits::IsEmpty(extractResult));
	}

	PACKET_SINGLE_ENTITY_TEST(CanExtractSingleBlockWithoutTransactions) {
		// Arrange: create a packet containing a block with no transactions
		ByteBuffer buffer(Block_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);

		// Act:
		auto extractResult = TTraits::Extract(packet);
		const model::Block* pBlock = nullptr;
		TTraits::Unwrap(extractResult, pBlock);

		// Assert:
		ASSERT_TRUE(!!pBlock);
		ASSERT_EQ(sizeof(model::BlockHeader), pBlock->Size);
		EXPECT_EQ_MEMORY(&buffer[sizeof(Packet)], pBlock, pBlock->Size);
	}

	PACKET_SINGLE_ENTITY_TEST(IsValidPredicateHasHigherPrecedenceThanSizeCheck) {
		// Arrange: create a packet containing a block with no transactions
		ByteBuffer buffer(Block_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);

		// Act: extract and return false from the isValid predicate even though the packet has a valid size
		auto numValidCalls = 0u;
		auto extractResult = TTraits::Extract(packet, [&numValidCalls](const auto&) {
			++numValidCalls;
			return false;
		});

		// Assert: valid was called once and extraction failed
		EXPECT_EQ(1u, numValidCalls);
		EXPECT_TRUE(TTraits::IsEmpty(extractResult));
	}

	PACKET_SINGLE_ENTITY_TEST(CanExtractSingleBlockWithTransaction) {
		// Arrange: create a packet containing a block with one transaction
		ByteBuffer buffer(Block_Transaction_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);
		SetTransactionAt(buffer, static_cast<uint32_t>(buffer.size() - Transaction_Size));

		// Act:
		auto extractResult = TTraits::Extract(packet);
		const model::Block* pBlock = nullptr;
		TTraits::Unwrap(extractResult, pBlock);

		// Assert:
		ASSERT_TRUE(!!pBlock);
		ASSERT_EQ(Block_Transaction_Size, pBlock->Size);
		EXPECT_EQ_MEMORY(&buffer[sizeof(Packet)], pBlock, pBlock->Size);
	}

	namespace {
		const Packet& PrepareMultiBlockPacket(ByteBuffer& buffer) {
			// create a packet containing three blocks
			buffer.resize(Block_Transaction_Packet_Size + 2 * sizeof(model::BlockHeader));
			const auto& packet = test::SetPushBlockPacketInBuffer(buffer);
			test::SetBlockAt(buffer, sizeof(Packet)); // block 1
			test::SetBlockAt(buffer, sizeof(Packet) + sizeof(model::BlockHeader), Block_Transaction_Size); // block 2
			SetTransactionAt(buffer, sizeof(Packet) + 2 * sizeof(model::BlockHeader)); // block 2 tx
			test::SetBlockAt(buffer, sizeof(Packet) + sizeof(model::BlockHeader) + Block_Transaction_Size); // block 3
			return packet;
		}
	}

	TEST(TEST_CLASS, CanExtractMultipleBlocks_ExtractEntities) {
		// Arrange: create a packet containing three blocks
		ByteBuffer buffer;
		const auto& packet = PrepareMultiBlockPacket(buffer);

		// Act:
		auto range = ExtractEntitiesFromPacket<model::Block>(packet, test::DefaultSizeCheck<model::Block>);

		// Assert:
		ASSERT_EQ(3u, range.size());

		// - block 1
		auto iter = range.cbegin();
		const auto* pBlock = &*iter;
		size_t offset = sizeof(Packet);
		ASSERT_EQ(sizeof(model::BlockHeader), pBlock->Size);
		EXPECT_EQ_MEMORY(&buffer[offset], pBlock, pBlock->Size);
		EXPECT_EQ(0u, reinterpret_cast<uintptr_t>(pBlock) % 8);

		// - block 2
		pBlock = &*++iter;
		offset += sizeof(model::BlockHeader);
		ASSERT_EQ(Block_Transaction_Size, pBlock->Size);
		EXPECT_EQ_MEMORY(&buffer[offset], pBlock, pBlock->Size);
		EXPECT_EQ(0u, reinterpret_cast<uintptr_t>(pBlock) % 8);

		// - block 3
		pBlock = &*++iter;
		offset += Block_Transaction_Size;
		ASSERT_EQ(sizeof(model::BlockHeader), pBlock->Size);
		EXPECT_EQ_MEMORY(&buffer[offset], pBlock, pBlock->Size);
		EXPECT_EQ(0u, reinterpret_cast<uintptr_t>(pBlock) % 8);
	}

	TEST(TEST_CLASS, CannotExtractMultipleBlocks_ExtractEntity) {
		// Arrange: create a packet containing three blocks
		ByteBuffer buffer;
		const auto& packet = PrepareMultiBlockPacket(buffer);

		// Act:
		auto pBlock = ExtractEntityFromPacket<model::Block>(packet, test::DefaultSizeCheck<model::Block>);

		// Assert:
		EXPECT_FALSE(!!pBlock);
	}

	// endregion

	// region ExtractFixedSizeStructuresFromPacket

	namespace {
		constexpr auto Fixed_Size = 62;
		using FixedSizeStructure = std::array<uint8_t, Fixed_Size>;

		void AssertCannotExtractFixedSizeStructuresFromPacketWithSize(uint32_t size) {
			// Arrange:
			auto buffer = test::GenerateRandomVector(size);
			auto& packet = reinterpret_cast<Packet&>(buffer[0]);
			packet.Size = size;

			// Act:
			auto range = ExtractFixedSizeStructuresFromPacket<FixedSizeStructure>(packet);

			// Assert:
			EXPECT_TRUE(range.empty()) << "packet size " << size;
		}
	}

	TEST(TEST_CLASS, CannotExtractFromPacketWithPartialStructures_FixedSizeStructures) {
		AssertCannotExtractFixedSizeStructuresFromPacketWithSize(sizeof(Packet) + 1);
		AssertCannotExtractFixedSizeStructuresFromPacketWithSize(sizeof(Packet) + Fixed_Size - 1);
		AssertCannotExtractFixedSizeStructuresFromPacketWithSize(sizeof(Packet) + Fixed_Size + 1);
		AssertCannotExtractFixedSizeStructuresFromPacketWithSize(sizeof(Packet) + 3 * Fixed_Size - 1);
		AssertCannotExtractFixedSizeStructuresFromPacketWithSize(sizeof(Packet) + 3 * Fixed_Size + 1);
	}

	TEST(TEST_CLASS, CanExtractSingleStructure_FixedSizeStructures) {
		// Arrange: create a packet containing a single fixed size structure
		constexpr auto Packet_Size = sizeof(Packet) + Fixed_Size;
		auto buffer = test::GenerateRandomVector(Packet_Size);
		auto& packet = reinterpret_cast<Packet&>(buffer[0]);
		packet.Size = Packet_Size;

		// Act:
		auto range = ExtractFixedSizeStructuresFromPacket<FixedSizeStructure>(packet);

		// Assert:
		ASSERT_EQ(1u, range.size());
		const auto& structure = *range.cbegin();
		EXPECT_EQ_MEMORY(&buffer[sizeof(Packet)], &structure, Fixed_Size);
	}

	TEST(TEST_CLASS, CanExtractMultipleStructures_FixedSizeStructures) {
		// Arrange: create a packet containing three fixed size structures
		constexpr auto Packet_Size = sizeof(Packet) + 3 * Fixed_Size;
		auto buffer = test::GenerateRandomVector(Packet_Size);
		auto& packet = reinterpret_cast<Packet&>(buffer[0]);
		packet.Size = Packet_Size;

		// Act:
		auto range = ExtractFixedSizeStructuresFromPacket<FixedSizeStructure>(packet);

		// Assert:
		ASSERT_EQ(3u, range.size());

		// - structure 1
		auto iter = range.cbegin();
		size_t offset = sizeof(Packet);
		EXPECT_EQ_MEMORY(&buffer[offset], &*iter, Fixed_Size);

		// - structure 2
		++iter;
		offset += Fixed_Size;
		EXPECT_EQ_MEMORY(&buffer[offset], &*iter, Fixed_Size);

		// - structure 3
		++iter;
		offset += Fixed_Size;
		EXPECT_EQ_MEMORY(&buffer[offset], &*iter, Fixed_Size);
	}

	// endregion
}}
