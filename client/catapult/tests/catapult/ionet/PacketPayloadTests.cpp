#include "catapult/ionet/PacketPayload.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketPayloadTests

	namespace {
		constexpr auto Test_Packet_Type = static_cast<PacketType>(987);
	}

	TEST(TEST_CLASS, CanCreateUnsetPacketPayload) {
		// Act:
		PacketPayload payload;

		// Assert:
		EXPECT_TRUE(payload.unset());
		EXPECT_EQ(0, payload.header().Size);
		EXPECT_EQ(PacketType::Undefined, payload.header().Type);
		EXPECT_TRUE(payload.buffers().empty());
	}

	TEST(TEST_CLASS, CanCreatePacketPayloadWithNoDataBuffers) {
		// Act:
		auto payload = PacketPayload(PacketType::Push_Block);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Push_Block);
		EXPECT_TRUE(payload.buffers().empty());
	}

	namespace {
		struct SharedTraits {
			static auto CreatePacketPointer(uint32_t payloadSize) {
				uint32_t packetSize = sizeof(PacketHeader) + payloadSize;
				return std::shared_ptr<Packet>(reinterpret_cast<Packet*>(::operator new (packetSize)));
			}
		};

		struct UniqueTraits {
			static auto CreatePacketPointer(uint32_t payloadSize) {
				uint32_t packetSize = sizeof(PacketHeader) + payloadSize;
				return std::unique_ptr<Packet>(reinterpret_cast<Packet*>(::operator new (packetSize)));
			}
		};
	}

#define PACKET_PAYLOAD_POINTER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Shared) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SharedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Unique) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UniqueTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	PACKET_PAYLOAD_POINTER_TEST(CannotCreatePacketPayloadFromPacketWithInvalidData) {
		// Arrange:
		auto pPacket = TTraits::CreatePacketPointer(0);
		pPacket->Size = sizeof(PacketHeader) - 1;
		pPacket->Type = Test_Packet_Type;

		// Act:
		EXPECT_THROW(PacketPayload(std::move(pPacket)), catapult_invalid_argument);
	}

	PACKET_PAYLOAD_POINTER_TEST(CanCreatePacketPayloadFromPacketWithNoData) {
		// Arrange:
		auto pPacket = TTraits::CreatePacketPointer(0);
		pPacket->Size = sizeof(PacketHeader);
		pPacket->Type = Test_Packet_Type;

		// Act:
		auto payload = PacketPayload(std::move(pPacket));
		pPacket.reset();

		// Sanity:
		EXPECT_FALSE(!!pPacket);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), Test_Packet_Type);
		EXPECT_TRUE(payload.buffers().empty());
	}

	PACKET_PAYLOAD_POINTER_TEST(CanCreatePacketPayloadFromPacketWithData) {
		// Arrange:
		constexpr auto Data_Size = 123u;
		auto data = test::GenerateRandomData<Data_Size>();
		auto pPacket = TTraits::CreatePacketPointer(Data_Size);
		pPacket->Size = sizeof(PacketHeader) + Data_Size;
		pPacket->Type = Test_Packet_Type;
		std::memcpy(pPacket->Data(), data.data(), Data_Size);

		// Act:
		auto payload = PacketPayload(std::move(pPacket));
		pPacket.reset();

		// Sanity:
		EXPECT_FALSE(!!pPacket);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + Data_Size, Test_Packet_Type);
		ASSERT_EQ(1u, payload.buffers().size());

		const auto& buffer = payload.buffers()[0];
		ASSERT_EQ(Data_Size, buffer.Size);
		EXPECT_TRUE(0 == std::memcmp(data.data(), buffer.pData, Data_Size));
	}

	namespace {
		template<typename TEntity>
		std::shared_ptr<TEntity> MakeEntityWithSizeT(uint32_t size) {
			using NonConstEntityType = std::remove_const_t<TEntity>;
			std::unique_ptr<NonConstEntityType> pEntity(reinterpret_cast<NonConstEntityType*>(::operator new (size)));
			pEntity->Size = size;

			auto headerSize = model::VerifiableEntity::Header_Size;
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pEntity.get()) + headerSize, size - headerSize });
			return std::move(pEntity);
		}

		template<typename TEntity>
		struct EntityTraits {
			using EntityPointerType = std::shared_ptr<TEntity>;
			using EntitiesContainer = std::vector<EntityPointerType>;

			static EntityPointerType MakeEntityWithSize(uint32_t size) {
				return MakeEntityWithSizeT<TEntity>(size);
			}
		};

		template<typename TEntityTraits>
		static typename TEntityTraits::EntitiesContainer MakeEntitiesWithSizes(const std::vector<uint32_t>& sizes) {
			typename TEntityTraits::EntitiesContainer entities;
			for (auto size : sizes)
				entities.push_back(TEntityTraits::MakeEntityWithSize(size));

			return entities;
		}

		using ConstEntityTraits = EntityTraits<const model::VerifiableEntity>;
		using NonConstEntityTraits = EntityTraits<model::VerifiableEntity>;
	}

	// region FromEntity / FromEntities

#define CONST_NON_CONST_ENTITY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonConstEntityTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CONST_NON_CONST_ENTITY_TEST(CanCreatePacketFromZeroEntities) {
		// Arrange:
		typename TTraits::EntitiesContainer entities;

		// Act:
		auto payload = PacketPayload::FromEntities(Test_Packet_Type, entities);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), Test_Packet_Type);
		EXPECT_TRUE(payload.buffers().empty());
	}

	CONST_NON_CONST_ENTITY_TEST(CanCreatePacketFromSingleEntity) {
		// Arrange:
		auto pEntity = TTraits::MakeEntityWithSize(124);

		// Act:
		auto payload = PacketPayload::FromEntity(Test_Packet_Type, pEntity);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 124u, Test_Packet_Type);
		ASSERT_EQ(1u, payload.buffers().size());

		// - the buffer contains the correct data and points to the original entity
		auto buffer = payload.buffers()[0];
		EXPECT_EQ(test::AsVoidPointer(pEntity.get()), buffer.pData);
		EXPECT_EQ(124u, buffer.Size);
		EXPECT_EQ(*pEntity, *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
	}

	TEST(TEST_CLASS, CreatingPacketFromSingleEntityExtendsEntityLifetime) {
		// Arrange:
		auto pEntity = NonConstEntityTraits::MakeEntityWithSize(124);
		auto pCopyEntity = test::CopyEntity(*pEntity);

		// Act:
		auto payload = PacketPayload::FromEntity(Test_Packet_Type, pEntity);
		pEntity.reset();

		// Sanity:
		EXPECT_FALSE(!!pEntity);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 124u, Test_Packet_Type);
		ASSERT_EQ(1u, payload.buffers().size());

		// - the buffer still has the original data even though the original entity was deleted
		auto buffer = payload.buffers()[0];
		EXPECT_EQ(*pCopyEntity, *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
	}

	TEST(TEST_CLASS, CannotCreatePacketFromSingleEntityWithOverflowingSize) {
		// Arrange:
		auto pEntity = NonConstEntityTraits::MakeEntityWithSize(124);
		pEntity->Size = 0xFFFF'FFF8;

		// Act:
		EXPECT_THROW(PacketPayload::FromEntity(Test_Packet_Type, pEntity), catapult_runtime_error);
	}

	CONST_NON_CONST_ENTITY_TEST(CanCreatePacketFromMultipleEntities) {
		// Arrange:
		auto entities = MakeEntitiesWithSizes<TTraits>({ 124, 300, 198 });

		// Act:
		auto payload = PacketPayload::FromEntities(Test_Packet_Type, entities);

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 622u, Test_Packet_Type);
		ASSERT_EQ(3u, payload.buffers().size());

		// - the buffers contain the correct data and point to the original entities
		for (auto i = 0u; i < payload.buffers().size(); ++i) {
			const auto& buffer = payload.buffers()[i];
			EXPECT_EQ(test::AsVoidPointer(entities[i].get()), buffer.pData);
			EXPECT_EQ(entities[i]->Size, buffer.Size);
			EXPECT_EQ(*entities[i], *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
		}
	}

	TEST(TEST_CLASS, CreatingPacketFromMultipleEntitiesExtendsEntityLifetimes) {
		// Arrange:
		auto entities = MakeEntitiesWithSizes<NonConstEntityTraits>({ 124, 300, 198 });

		NonConstEntityTraits::EntitiesContainer copyEntities;
		for (auto pEntity : entities)
			copyEntities.push_back(test::CopyEntity(*pEntity));

		// Act:
		auto payload = PacketPayload::FromEntities(Test_Packet_Type, entities);
		entities.clear();

		// Sanity:
		EXPECT_TRUE(entities.empty());

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 622u, Test_Packet_Type);
		ASSERT_EQ(3u, payload.buffers().size());

		// - the buffers still have the original data even though the original entities were deleted
		for (auto i = 0u; i < payload.buffers().size(); ++i) {
			const auto& buffer = payload.buffers()[i];
			EXPECT_EQ(*copyEntities[i], *reinterpret_cast<const model::VerifiableEntity*>(buffer.pData));
		}
	}

	TEST(TEST_CLASS, CannotCreatePacketFromMultipleEntitiesWithOverflowingSize) {
		// Arrange:
		auto entities = MakeEntitiesWithSizes<NonConstEntityTraits>({ 124, 300, 198 });

		// - make calculated total size too large (3 * 0x55555553 + 0x08 > 0xFFFFFFFF)
		for (auto pEntity : entities)
			pEntity->Size = 0xFFFF'FFFA / 3;

		// Act:
		EXPECT_THROW(PacketPayload::FromEntities(Test_Packet_Type, entities), catapult_runtime_error);
	}

	// endregion

	// region FromFixedSizeRange

	TEST(TEST_CLASS, CanCreatePacketFromEmptyFixedSizeRange) {
		// Arrange:
		model::EntityRange<uint32_t> range;

		// Act:
		auto payload = PacketPayload::FromFixedSizeRange(Test_Packet_Type, std::move(range));

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), Test_Packet_Type);
		EXPECT_TRUE(payload.buffers().empty());
	}

	namespace {
		constexpr std::array<uint8_t, 12> Entity_Range_Buffer{{
			0x00, 0x11, 0x22, 0x33,
			0xFF, 0xDD, 0xBB, 0x99,
			0x76, 0x98, 0x12, 0x34,
		}};
	}

	TEST(TEST_CLASS, CanCreatePacketFromFixedSizeRange) {
		// Arrange:
		auto range = model::EntityRange<uint32_t>::CopyFixed(Entity_Range_Buffer.data(), 3);

		// Act:
		auto payload = PacketPayload::FromFixedSizeRange(Test_Packet_Type, std::move(range));

		// Sanity:
		EXPECT_TRUE(range.empty());

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 12u, Test_Packet_Type);
		ASSERT_EQ(1u, payload.buffers().size());

		auto buffer = payload.buffers()[0];
		EXPECT_EQ(12u, buffer.Size);
		EXPECT_TRUE(0 == std::memcmp(Entity_Range_Buffer.data(), buffer.pData, buffer.Size));
	}

	TEST(TEST_CLASS, CanCreatePacketFromOverlaidFixedSizeRange) {
		// Arrange:
		auto range = model::EntityRange<uint32_t>::CopyVariable(
				Entity_Range_Buffer.data(),
				Entity_Range_Buffer.size(),
				{ 2, 6 });

		// Act:
		auto payload = PacketPayload::FromFixedSizeRange(Test_Packet_Type, std::move(range));

		// Sanity:
		EXPECT_TRUE(range.empty());

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 8u, Test_Packet_Type);
		ASSERT_EQ(1u, payload.buffers().size());

		auto buffer = payload.buffers()[0];
		EXPECT_EQ(8u, buffer.Size);
		EXPECT_TRUE(0 == std::memcmp(Entity_Range_Buffer.data() + 2u, buffer.pData, buffer.Size));
	}

	// endregion
}}
