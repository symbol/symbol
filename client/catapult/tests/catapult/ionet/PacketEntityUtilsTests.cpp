#include "catapult/ionet/PacketEntityUtils.h"
#include "catapult/ionet/IoTypes.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketEntityUtilsTests

	// region ExtractEntitiesFromPacket / ExtractEntityFromPacket

	namespace {
		constexpr uint32_t Transaction_Size = sizeof(mocks::MockTransaction);
		constexpr uint32_t Block_Packet_Size = sizeof(Packet) + sizeof(model::Block);
		constexpr uint32_t Block_Transaction_Size = sizeof(model::Block) + Transaction_Size;
		constexpr uint32_t Block_Transaction_Packet_Size = sizeof(Packet) + Block_Transaction_Size;

		template<typename TEntity>
		bool DefaultSizeCheck(const TEntity& entity) {
			auto pRegistry = mocks::CreateDefaultTransactionRegistry();
			return IsSizeValid(entity, *pRegistry);
		}

		void SetTransactionAt(ByteBuffer& buffer, size_t offset) {
			test::SetTransactionAt(buffer, offset, Transaction_Size);
		}

		struct ExtractEntityTraits {
			template<typename TIsValidPredicate>
			static auto Extract(const Packet& packet, TIsValidPredicate isValid) {
				return ExtractEntityFromPacket<model::Block>(packet, isValid);
			}

			static auto Extract(const Packet& packet) {
				return Extract(packet, DefaultSizeCheck<model::Block>);
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
				constexpr auto Base_Packet_Size = sizeof(Packet);
				buffer.resize(Base_Packet_Size + std::max<uint32_t>(sizeof(model::Block), size));
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
				return Extract(packet, DefaultSizeCheck<model::Block>);
			}

			static bool IsEmpty(const model::BlockRange& range) {
				return range.empty();
			}

			static void Unwrap(const model::BlockRange& range, const model::Block*& pBlockOut) {
				ASSERT_EQ(1u, range.size());
				pBlockOut = &*range.cbegin();
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
				constexpr auto Base_Packet_Size = sizeof(Packet) + Num_Full_Blocks * sizeof(model::Block);
				buffer.resize(Base_Packet_Size + std::max<uint32_t>(sizeof(model::Block), size));
				auto& packet = test::SetPushBlockPacketInBuffer(buffer);
				for (auto i = 0u; i <= Num_Full_Blocks; ++i) {
					auto blockSize = Num_Full_Blocks == i ? size : sizeof(model::Block);
					test::SetBlockAt(buffer, sizeof(Packet) + i * sizeof(model::Block), blockSize);
				}

				packet.Size = Base_Packet_Size + size;
				return packet;
			}
		};

		struct ExtractFixedSizeEntitiesTraits {
			static auto Extract(const Packet& packet) {
				return ExtractFixedSizeEntitiesFromPacket<Hash256>(packet);
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
			packet.Type = PacketType::Push_Block;

			// Act:
			auto extractedResult = TTraits::Extract(packet);

			// Assert:
			EXPECT_TRUE(TTraits::IsEmpty(extractedResult)) << "packet size " << size;
		}

		template<typename TTraits>
		void AssertCannotExtractEntitiesFromPacketWithSize(uint32_t size) {
			// Arrange:
			ByteBuffer buffer;
			const auto& packet = TTraits::CreatePacketWithOverflowSize(buffer, size);

			// Act:
			auto extractedResult = TTraits::Extract(packet);

			// Assert:
			EXPECT_TRUE(TTraits::IsEmpty(extractedResult)) << "overflow size " << size;
		}
	}

#define PACKET_HEADER_FAILURE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntities) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntitiesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_FixedSizeEntities) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractFixedSizeEntitiesTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define PACKET_FAILURE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntities_Single) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntitiesSingleEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntities_Last) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntitiesMultiEntityTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define PACKET_SINGLE_ENTITY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ExtractEntities) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ExtractEntitiesTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	PACKET_HEADER_FAILURE_TEST(CannotExtractFromPacketWithInvalidSize) {
		// Assert:
		AssertCannotExtractFromPacketHeaderWithNoData<TTraits>(sizeof(Packet) - 1);
	}

	PACKET_HEADER_FAILURE_TEST(CannotExtractFromPacketWithNoData) {
		// Assert:
		AssertCannotExtractFromPacketHeaderWithNoData<TTraits>(sizeof(Packet));
	}

	PACKET_FAILURE_TEST(CannotExtractFromPacketWithoutFullEntityHeader) {
		// Assert:
		std::vector<uint32_t> sizes{ 1, sizeof(model::VerifiableEntity) - 1 };
		for (auto size : sizes)
			AssertCannotExtractEntitiesFromPacketWithSize<TTraits>(size);
	}

	PACKET_FAILURE_TEST(CannotExtractFromPacketWithoutFullEntityData) {
		// Assert:
		std::vector<uint32_t> sizes{ sizeof(model::VerifiableEntity), sizeof(model::Block) - 1 };
		for (auto size : sizes)
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
		auto extractedResult = TTraits::Extract(packet);

		// Assert:
		EXPECT_TRUE(TTraits::IsEmpty(extractedResult));
	}

	PACKET_FAILURE_TEST(CannotExtractFromPacketWhenPacketExpandsBeyondLastEntity) {
		// Arrange:
		// - create a packet wrapping a block
		// - expand the buffer by the size of a transaction so it looks like the packet expands beyond the last entity
		ByteBuffer buffer;
		TTraits::CreatePacketWithOverflowSize(buffer, sizeof(model::Block));
		buffer.resize(buffer.size() + Transaction_Size);
		auto& packet = reinterpret_cast<Packet&>(buffer[0]);
		packet.Size += Transaction_Size;

		// Act:
		auto extractedResult = TTraits::Extract(packet);

		// Assert:
		EXPECT_TRUE(TTraits::IsEmpty(extractedResult));
	}

	PACKET_SINGLE_ENTITY_TEST(CanExtractSingleBlockWithoutTransactions) {
		// Arrange: create a packet containing a block with no transactions
		ByteBuffer buffer(Block_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);

		// Act:
		auto extractedResult = TTraits::Extract(packet);
		const model::Block* pBlock = nullptr;
		TTraits::Unwrap(extractedResult, pBlock);

		// Assert:
		ASSERT_TRUE(!!pBlock);
		EXPECT_EQ(sizeof(model::Block), pBlock->Size);
		EXPECT_TRUE(0 == std::memcmp(&buffer[sizeof(Packet)], pBlock, pBlock->Size));
	}

	PACKET_SINGLE_ENTITY_TEST(IsValidPredicateHasHigherPrecedenceThanSizeCheck) {
		// Arrange: create a packet containing a block with no transactions
		ByteBuffer buffer(Block_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);

		// Act: extract and return false from the isValid predicate even though the packet has a valid size
		auto numValidCalls = 0;
		auto extractedResult = TTraits::Extract(packet, [&numValidCalls](const auto&) {
			++numValidCalls;
			return false;
		});

		// Assert: valid was called once and extraction failed
		EXPECT_EQ(1u, numValidCalls);
		EXPECT_TRUE(TTraits::IsEmpty(extractedResult));
	}

	PACKET_SINGLE_ENTITY_TEST(CanExtractSingleBlockWithTransaction) {
		// Arrange: create a packet containing a block with one transaction
		ByteBuffer buffer(Block_Transaction_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);
		SetTransactionAt(buffer, static_cast<uint32_t>(buffer.size() - Transaction_Size));

		// Act:
		auto extractedResult = TTraits::Extract(packet);
		const model::Block* pBlock = nullptr;
		TTraits::Unwrap(extractedResult, pBlock);

		// Assert:
		ASSERT_TRUE(!!pBlock);
		EXPECT_EQ(Block_Transaction_Size, pBlock->Size);
		EXPECT_TRUE(0 == std::memcmp(&buffer[sizeof(Packet)], pBlock, pBlock->Size));
	}

	namespace {
		const Packet& PrepareMultiBlockPacket(ByteBuffer& buffer) {
			// create a packet containing three blocks
			buffer.resize(Block_Transaction_Packet_Size + 2 * sizeof(model::Block));
			const auto& packet = test::SetPushBlockPacketInBuffer(buffer);
			test::SetBlockAt(buffer, sizeof(Packet)); // block 1
			test::SetBlockAt(buffer, sizeof(Packet) + sizeof(model::Block), Block_Transaction_Size); // block 2
			SetTransactionAt(buffer, sizeof(Packet) + 2 * sizeof(model::Block)); // block 2 tx
			test::SetBlockAt(buffer, sizeof(Packet) + sizeof(model::Block) + Block_Transaction_Size); // block 3
			return packet;
		}
	}

	TEST(TEST_CLASS, CanExtractMultipleBlocks_ExtractEntities) {
		// Arrange: create a packet containing three blocks
		ByteBuffer buffer;
		const auto& packet = PrepareMultiBlockPacket(buffer);

		// Act:
		auto range = ExtractEntitiesFromPacket<model::Block>(packet, DefaultSizeCheck<model::Block>);

		// Assert:
		ASSERT_EQ(3u, range.size());

		// - block 1
		auto iter = range.cbegin();
		const auto* pBlock = &*iter;
		size_t offset = sizeof(Packet);
		EXPECT_EQ(sizeof(model::Block), pBlock->Size);
		EXPECT_TRUE(0 == std::memcmp(&buffer[offset], pBlock, pBlock->Size));

		// - block 2
		pBlock = &*++iter;
		offset += sizeof(model::Block);
		EXPECT_EQ(Block_Transaction_Size, pBlock->Size);
		EXPECT_TRUE(0 == std::memcmp(&buffer[offset], pBlock, pBlock->Size));

		// - block 3
		pBlock = &*++iter;
		offset += Block_Transaction_Size;
		EXPECT_EQ(sizeof(model::Block), pBlock->Size);
		EXPECT_TRUE(0 == std::memcmp(&buffer[offset], pBlock, pBlock->Size));
	}

	TEST(TEST_CLASS, CannotExtractMultipleBlocks_ExtractEntity) {
		// Arrange: create a packet containing three blocks
		ByteBuffer buffer;
		const auto& packet = PrepareMultiBlockPacket(buffer);

		// Act:
		auto pBlock = ExtractEntityFromPacket<model::Block>(packet, DefaultSizeCheck<model::Block>);

		// Assert:
		EXPECT_FALSE(!!pBlock);
	}

	// endregion

	// region ExtractFixedSizeEntitiesFromPacket

	namespace {
		constexpr auto Fixed_Size = 62;
		using FixedSizeEntity = std::array<uint8_t, Fixed_Size>;

		void AssertCannotExtractFixedSizeEntitiesFromPacketWithSize(uint32_t size) {
			// Arrange:
			ByteBuffer buffer(size);
			test::FillWithRandomData(buffer);
			auto& packet = reinterpret_cast<Packet&>(*buffer.data());
			packet.Size = size;

			// Act:
			auto range = ExtractFixedSizeEntitiesFromPacket<FixedSizeEntity>(packet);

			// Assert:
			EXPECT_TRUE(range.empty()) << "packet size " << size;
		}
	}

	TEST(TEST_CLASS, CannotExtractFromPacketWithPartialEntities_FixedSizeEntities) {
		// Assert:
		AssertCannotExtractFixedSizeEntitiesFromPacketWithSize(sizeof(Packet) + 1);
		AssertCannotExtractFixedSizeEntitiesFromPacketWithSize(sizeof(Packet) + Fixed_Size - 1);
		AssertCannotExtractFixedSizeEntitiesFromPacketWithSize(sizeof(Packet) + Fixed_Size + 1);
		AssertCannotExtractFixedSizeEntitiesFromPacketWithSize(sizeof(Packet) + 3 * Fixed_Size - 1);
		AssertCannotExtractFixedSizeEntitiesFromPacketWithSize(sizeof(Packet) + 3 * Fixed_Size + 1);
	}

	TEST(TEST_CLASS, CanExtractSingleEntity_FixedSizeEntities) {
		// Arrange: create a packet containing a single fixed size entity
		constexpr auto Packet_Size = sizeof(Packet) + Fixed_Size;
		ByteBuffer buffer(Packet_Size);
		test::FillWithRandomData(buffer);
		auto& packet = reinterpret_cast<Packet&>(*buffer.data());
		packet.Size = Packet_Size;

		// Act:
		auto range = ExtractFixedSizeEntitiesFromPacket<FixedSizeEntity>(packet);

		// Assert:
		ASSERT_EQ(1u, range.size());
		const auto& entity = *range.cbegin();
		EXPECT_TRUE(0 == std::memcmp(&buffer[sizeof(Packet)], &entity, Fixed_Size));
	}

	TEST(TEST_CLASS, CanExtractMultipleEntities_FixedSizeEntities) {
		// Arrange: create a packet containing three fixed size entities
		constexpr auto Packet_Size = sizeof(Packet) + 3 * Fixed_Size;
		ByteBuffer buffer(Packet_Size);
		test::FillWithRandomData(buffer);
		auto& packet = reinterpret_cast<Packet&>(*buffer.data());
		packet.Size = Packet_Size;

		// Act:
		auto range = ExtractFixedSizeEntitiesFromPacket<FixedSizeEntity>(packet);

		// Assert:
		ASSERT_EQ(3u, range.size());

		// - entity 1
		auto iter = range.cbegin();
		size_t offset = sizeof(Packet);
		EXPECT_TRUE(0 == std::memcmp(&buffer[offset], &*iter, Fixed_Size));

		// - entity 2
		++iter;
		offset += Fixed_Size;
		EXPECT_TRUE(0 == std::memcmp(&buffer[offset], &*iter, Fixed_Size));

		// - entity 3
		++iter;
		offset += Fixed_Size;
		EXPECT_TRUE(0 == std::memcmp(&buffer[offset], &*iter, Fixed_Size));
	}

	// endregion
}}
