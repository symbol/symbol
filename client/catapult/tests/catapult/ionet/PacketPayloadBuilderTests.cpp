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

#include "catapult/ionet/PacketPayloadBuilder.h"
#include "catapult/model/VerifiableEntity.h"
#include "catapult/utils/Functional.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketPayloadBuilderTests

	// region empty

	TEST(TEST_CLASS, CanBuildEmptyPayload) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);

		// Act:
		auto payload = builder.build();

		// Assert:
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Chain_Statistics);
		EXPECT_TRUE(payload.buffers().empty());
	}

	// endregion

	namespace {
		using Buffers = std::vector<RawBuffer>;
		using EntitiesContainer = std::vector<std::shared_ptr<model::VerifiableEntity>>;

		std::shared_ptr<model::VerifiableEntity> MakeEntityWithSize(uint32_t size) {
			return test::CreateRandomEntityWithSize<>(size);
		}
	}

	// region traits

	namespace {
		enum class AssertBuffersType { Deep, Shallow };

		struct SingleEntityTraits {
			using DataType = std::shared_ptr<model::VerifiableEntity>;

			static auto CreateAppendData() {
				return MakeEntityWithSize(124);
			}

			static uint32_t GetDataSize(const DataType& pEntity) {
				return pEntity->Size;
			}

			static bool Append(PacketPayloadBuilder& builder, const DataType& pEntity) {
				return builder.appendEntity(pEntity);
			}

			static void AssertBuffers(
					const Buffers& buffers,
					const DataType& pEntity,
					AssertBuffersType assertType = AssertBuffersType::Deep) {
				// Assert:
				ASSERT_EQ(1u, buffers.size());

				// - the buffer contains the correct data and points to the original entity (if Deep)
				auto buffer = buffers[0];
				if (AssertBuffersType::Deep == assertType)
					ASSERT_EQ(test::AsVoidPointer(pEntity.get()), buffer.pData);

				ASSERT_EQ(pEntity->Size, buffer.Size);
				EXPECT_EQ(*pEntity, reinterpret_cast<const model::VerifiableEntity&>(*buffer.pData));
			}

			static DataType CopyData(const DataType& pEntity) {
				return test::CopyEntity(*pEntity);
			}

			/* DEFINE_OVERFLOW_APPEND_TESTS */

			static void MakeOverflow(DataType& pEntity) {
				// make calculated total size too large (0xFFFF'FFF8 + 0x08 > 0xFFFFFFFF)
				pEntity->Size = 0xFFFF'FFF8;
			}
		};

		struct MultipleEntitiesTraits {
			using DataType = EntitiesContainer;

			static auto CreateAppendData() {
				return EntitiesContainer{ MakeEntityWithSize(124), MakeEntityWithSize(300), MakeEntityWithSize(198) };
			}

			static uint32_t GetDataSize(const DataType& entities) {
				return utils::Sum(entities, [](const auto& pEntity) { return pEntity->Size; });
			}

			static bool Append(PacketPayloadBuilder& builder, const DataType& entities) {
				return builder.appendEntities(entities);
			}

			static void AssertBuffers(
					const Buffers& buffers,
					const DataType& entities,
					AssertBuffersType assertType = AssertBuffersType::Deep) {
				// Assert:
				ASSERT_EQ(entities.size(), buffers.size());

				// - the buffers contain the correct data and point to the original entities (if Deep)
				for (auto i = 0u; i < buffers.size(); ++i) {
					const auto& buffer = buffers[i];
					auto message = "buffer at " + std::to_string(i);
					if (AssertBuffersType::Deep == assertType)
						ASSERT_EQ(test::AsVoidPointer(entities[i].get()), buffer.pData) << message;

					ASSERT_EQ(entities[i]->Size, buffer.Size) << message;
					EXPECT_EQ(*entities[i], reinterpret_cast<const model::VerifiableEntity&>(*buffer.pData)) << message;
				}
			}

			static DataType CopyData(const DataType& entities) {
				EntitiesContainer copyEntities;
				for (const auto& pEntity : entities)
					copyEntities.push_back(test::CopyEntity(*pEntity));

				return copyEntities;
			}

			/* DEFINE_OVERFLOW_APPEND_TESTS */

			static void MakeOverflow(DataType& entities) {
				// make calculated total size too large (0xFFFF'FFFA + 0x08 > 0xFFFFFFFF)
				for (auto& pEntity : entities)
					pEntity->Size = static_cast<uint32_t>(0xFFFF'FFFA / entities.size());
			}
		};

		struct EntitiesGeneratorTraits {
			struct DataType {
			public:
				DataType()
						: Index(0)
						, Entities(MultipleEntitiesTraits::CreateAppendData())
				{}

			public:
				auto operator()() {
					return Index >= Entities.size() ? nullptr : Entities[Index++];
				}

			public:
				size_t Index;
				EntitiesContainer Entities;
			};

			static auto CreateAppendData() {
				return DataType();
			}

			static uint32_t GetDataSize(const DataType& generator) {
				return MultipleEntitiesTraits::GetDataSize(generator.Entities);
			}

			static bool Append(PacketPayloadBuilder& builder, const DataType& generator) {
				return builder.appendGeneratedEntities(DataType(generator));
			}

			static void AssertBuffers(
					const Buffers& buffers,
					const DataType& generator,
					AssertBuffersType assertType = AssertBuffersType::Deep) {
				return MultipleEntitiesTraits::AssertBuffers(buffers, generator.Entities, assertType);
			}

			static DataType CopyData(const DataType& generator) {
				DataType copy;
				copy.Entities = MultipleEntitiesTraits::CopyData(generator.Entities);
				return copy;
			}

			/* DEFINE_OVERFLOW_APPEND_TESTS */

			static void MakeOverflow(DataType& generator) {
				MultipleEntitiesTraits::MakeOverflow(generator.Entities);
			}
		};

		struct EntityRangeTraits {
			using DataType = model::EntityRange<uint32_t>;

			static auto CreateAppendData() {
				auto buffer = test::GenerateRandomVector(3 * sizeof(uint32_t));
				return model::EntityRange<uint32_t>::CopyFixed(buffer.data(), 3);
			}

			static uint32_t GetDataSize(const DataType& range) {
				return static_cast<uint32_t>(range.size() * sizeof(uint32_t));
			}

			static bool Append(PacketPayloadBuilder& builder, const DataType& range) {
				return builder.appendRange(CopyData(range));
			}

			static void AssertBuffers(const Buffers& buffers, const DataType& range, AssertBuffersType = AssertBuffersType::Deep) {
				// Assert:
				ASSERT_EQ(1u, buffers.size());

				auto buffer = buffers[0];
				ASSERT_EQ(GetDataSize(range), buffer.Size);

				auto rangeIter = range.cbegin();
				const auto* pValue = reinterpret_cast<const uint32_t*>(buffer.pData);
				for (auto i = 0u; i < range.size(); ++i, ++rangeIter, ++pValue)
					EXPECT_EQ(*rangeIter, *pValue) << "value at " << i;
			}

			static DataType CopyData(const DataType& range) {
				return model::EntityRange<uint32_t>::CopyFixed(reinterpret_cast<const uint8_t*>(range.data()), range.size());
			}
		};

		struct ValueTraits {
			using DataType = Hash256;

			static auto CreateAppendData() {
				return test::GenerateRandomByteArray<Hash256>();
			}

			static uint32_t GetDataSize(const DataType&) {
				return Hash256::Size;
			}

			static bool Append(PacketPayloadBuilder& builder, const DataType& value) {
				return builder.appendValue(value);
			}

			static void AssertBuffers(const Buffers& buffers, const DataType& value, AssertBuffersType = AssertBuffersType::Deep) {
				// Assert:
				ASSERT_EQ(1u, buffers.size());

				auto buffer = buffers[0];
				ASSERT_EQ(Hash256::Size, buffer.Size);
				EXPECT_EQ(value, reinterpret_cast<const Hash256&>(*buffer.pData));
			}

			static DataType CopyData(const DataType& value) {
				return value;
			}
		};

		struct ValuesTraits {
			using DataType = std::vector<Hash256>;

			static auto CreateAppendData() {
				return test::GenerateRandomDataVector<Hash256>(3);
			}

			static uint32_t GetDataSize(const DataType& values) {
				return static_cast<uint32_t>(values.size() * Hash256::Size);
			}

			static bool Append(PacketPayloadBuilder& builder, const DataType& values) {
				return builder.appendValues(values);
			}

			static void AssertBuffers(const Buffers& buffers, const DataType& values, AssertBuffersType = AssertBuffersType::Deep) {
				// Assert:
				ASSERT_EQ(1u, buffers.size());

				auto buffer = buffers[0];
				ASSERT_EQ(GetDataSize(values), buffer.Size);

				const auto* pHash = reinterpret_cast<const Hash256*>(buffer.pData);
				for (auto i = 0u; i < values.size(); ++i, ++pHash)
					EXPECT_EQ(values[i], *pHash) << "value at " << i;
			}

			static DataType CopyData(const DataType& values) {
				return values;
			}
		};

		struct ValuesGeneratorTraits {
			struct DataType {
			public:
				DataType()
						: Index(0)
						, Values(ValuesTraits::CreateAppendData())
				{}

			public:
				auto operator()() {
					return Index >= Values.size() ? nullptr : &Values[Index++];
				}

			public:
				size_t Index;
				std::vector<Hash256> Values;
			};

			static auto CreateAppendData() {
				return DataType();
			}

			static uint32_t GetDataSize(const DataType& generator) {
				return ValuesTraits::GetDataSize(generator.Values);
			}

			static bool Append(PacketPayloadBuilder& builder, const DataType& generator) {
				return builder.appendGeneratedValues(DataType(generator));
			}

			static void AssertBuffers(
					const Buffers& buffers,
					const DataType& generator,
					AssertBuffersType assertType = AssertBuffersType::Deep) {
				return ValuesTraits::AssertBuffers(buffers, generator.Values, assertType);
			}

			static DataType CopyData(const DataType& generator) {
				return generator;
			}
		};
	}

	// endregion

#define DEFINE_BASIC_APPEND_TESTS(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_SingleEntity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SingleEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MultipleEntities) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MultipleEntitiesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_EntitiesGenerator) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EntitiesGeneratorTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_EntityRange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EntityRangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Value) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValueTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Values) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValuesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ValuesGenerator) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ValuesGeneratorTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define DEFINE_OVERFLOW_APPEND_TESTS(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_SingleEntity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SingleEntityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MultipleEntities) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MultipleEntitiesTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_EntitiesGenerator) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EntitiesGeneratorTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region basic append tests

	DEFINE_BASIC_APPEND_TESTS(CanAppendData) {
		// Arrange:
		auto appendData = TTraits::CreateAppendData();
		auto appendDataSize = TTraits::GetDataSize(appendData);
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);

		// Act:
		auto isAppendSuccess = TTraits::Append(builder, appendData);
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + appendDataSize, PacketType::Chain_Statistics);
		TTraits::AssertBuffers(payload.buffers(), appendData);
	}

	DEFINE_BASIC_APPEND_TESTS(CanAppendDataWithSizeEqualToLimit) {
		// Arrange:
		auto appendData = TTraits::CreateAppendData();
		auto appendDataSize = TTraits::GetDataSize(appendData);
		PacketPayloadBuilder builder(PacketType::Chain_Statistics, appendDataSize);

		// Act:
		auto isAppendSuccess = TTraits::Append(builder, appendData);
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + appendDataSize, PacketType::Chain_Statistics);
		TTraits::AssertBuffers(payload.buffers(), appendData);
	}

	DEFINE_BASIC_APPEND_TESTS(CannotAppendDataWithSizeExceedingLimit) {
		// Arrange:
		auto appendData = TTraits::CreateAppendData();
		auto appendDataSize = TTraits::GetDataSize(appendData);
		PacketPayloadBuilder builder(PacketType::Chain_Statistics, appendDataSize - 1);

		// Act:
		auto isAppendSuccess = TTraits::Append(builder, appendData);
		auto payload = builder.build();

		// Assert:
		EXPECT_FALSE(isAppendSuccess);
		EXPECT_TRUE(payload.unset());
	}

	DEFINE_BASIC_APPEND_TESTS(CannotAppendDataAfterAppendDataFailure) {
		// Arrange:
		auto appendData1 = TTraits::CreateAppendData();
		auto appendData2 = TTraits::CreateAppendData();
		auto appendDataSize = TTraits::GetDataSize(appendData1);
		PacketPayloadBuilder builder(PacketType::Chain_Statistics, appendDataSize + appendDataSize / 2);

		// Act: the second append should fail (sticky) causing the payload to be unset
		auto isAppendSuccess1 = TTraits::Append(builder, appendData1);
		auto isAppendSuccess2 = TTraits::Append(builder, appendData2);
		auto isAppendSuccess3 = TTraits::Append(builder, appendData1);
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess1);
		EXPECT_FALSE(isAppendSuccess2);
		EXPECT_FALSE(isAppendSuccess3);
		EXPECT_TRUE(payload.unset());
	}

	DEFINE_BASIC_APPEND_TESTS(AppendingDataExtendsDataLifetime) {
		// Arrange:
		typename TTraits::DataType appendDataCopy;
		size_t appendDataSize;
		bool isAppendSuccess;
		ionet::PacketPayload payload;

		// - ensure the appended data goes out of scope
		{
			auto appendData = TTraits::CreateAppendData();
			appendDataCopy = TTraits::CopyData(appendData);
			appendDataSize = TTraits::GetDataSize(appendData);

			// Act:
			PacketPayloadBuilder builder(PacketType::Chain_Statistics);
			isAppendSuccess = TTraits::Append(builder, appendData);
			payload = builder.build();
		}

		// Assert: use Shallow compare because copy is passed into AssertBuffers
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + appendDataSize, PacketType::Chain_Statistics);
		TTraits::AssertBuffers(payload.buffers(), appendDataCopy, AssertBuffersType::Shallow);
	}

	// endregion

	// region overflow append tests

	DEFINE_OVERFLOW_APPEND_TESTS(CannotAppendDataWithOverflowingSize) {
		// Arrange:
		auto appendData = TTraits::CreateAppendData();
		TTraits::MakeOverflow(appendData);
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);

		// Act:
		auto isAppendSuccess = TTraits::Append(builder, appendData);
		auto payload = builder.build();

		// Assert:
		EXPECT_FALSE(isAppendSuccess);
		EXPECT_TRUE(payload.unset());
	}

	// endregion

	// region (mutiple) entities

	TEST(TEST_CLASS, CanBuildPayloadAroundZeroEntities) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);

		// Act:
		auto isAppendSuccess = builder.appendEntities(EntitiesContainer());
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Chain_Statistics);
		EXPECT_TRUE(payload.buffers().empty());
	}

	TEST(TEST_CLASS, CanBuildPayloadAroundZeroGeneratedEntities) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);
		auto generator = []() { return std::shared_ptr<model::VerifiableEntity>(); };

		// Act:
		auto isAppendSuccess = builder.appendGeneratedEntities(generator);
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Chain_Statistics);
		EXPECT_TRUE(payload.buffers().empty());
	}

	// endregion

	// region range

	TEST(TEST_CLASS, CanAppendEmptyFixedSizeRange) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);

		// Act:
		auto isAppendSuccess = builder.appendRange(model::EntityRange<uint32_t>());
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Chain_Statistics);
		EXPECT_TRUE(payload.buffers().empty());
	}

	TEST(TEST_CLASS, CanAppendOverlaidFixedSizeRange) {
		// Arrange:
		auto rangeBuffer = test::GenerateRandomVector(3 * sizeof(uint32_t));
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);
		auto range = model::EntityRange<uint32_t>::CopyVariable(rangeBuffer.data(), rangeBuffer.size(), { 2, 6 });

		// Act:
		auto isAppendSuccess = builder.appendRange(std::move(range));
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 8u, PacketType::Chain_Statistics);
		ASSERT_EQ(1u, payload.buffers().size());

		auto buffer = payload.buffers()[0];
		ASSERT_EQ(8u, buffer.Size);
		EXPECT_EQ_MEMORY(rangeBuffer.data() + 2u, buffer.pData, buffer.Size);
	}

	// endregion

	// region value

	TEST(TEST_CLASS, CanAppendUint32) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);

		// Act:
		auto isAppendSuccess = builder.appendValue<uint32_t>(0x03981204);
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 4u, PacketType::Chain_Statistics);
		ASSERT_EQ(1u, payload.buffers().size());

		auto buffer = payload.buffers()[0];
		ASSERT_EQ(4u, buffer.Size);
		EXPECT_EQ(0x03981204u, reinterpret_cast<const uint32_t&>(*buffer.pData));
	}

	// endregion

	// region values

	TEST(TEST_CLASS, CanAppendZeroValues) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);

		// Act:
		auto isAppendSuccess = builder.appendValues(std::vector<Hash256>());
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Chain_Statistics);
		EXPECT_TRUE(payload.buffers().empty());
	}

	TEST(TEST_CLASS, CanAppendZeroGeneratedValues) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);
		auto generator = []() { return std::shared_ptr<Hash256>(); };

		// Act:
		auto isAppendSuccess = builder.appendGeneratedValues(generator);
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader), PacketType::Chain_Statistics);
		EXPECT_TRUE(payload.buffers().empty());
	}

	// endregion

	// region mixed

	TEST(TEST_CLASS, CanAppendHeterogeneousSources) {
		// Arrange:
		PacketPayloadBuilder builder(PacketType::Chain_Statistics);

		auto rangeBuffer = test::GenerateRandomVector(3 * sizeof(uint32_t));
		auto valuesBuffer = test::GenerateRandomVector(3 * sizeof(uint32_t));
		auto pEntity = MakeEntityWithSize(124);

		// Act:
		auto isAppendSuccess = true
				&& builder.appendValue<uint32_t>(0x03981204)
				&& builder.appendEntity(pEntity)
				&& builder.appendValues(valuesBuffer)
				&& builder.appendValue<uint32_t>(0x11111111)
				&& builder.appendRange(model::EntityRange<uint32_t>::CopyFixed(rangeBuffer.data(), 2))
				&& builder.appendValue<uint32_t>(0x00003322);
		auto payload = builder.build();

		// Assert:
		EXPECT_TRUE(isAppendSuccess);
		test::AssertPacketHeader(payload, sizeof(PacketHeader) + 3u * 4 + 124 + 8 + 12, PacketType::Chain_Statistics);
		ASSERT_EQ(6u, payload.buffers().size());

		const auto& buffers = payload.buffers();
		ASSERT_EQ(4u, buffers[0].Size);
		EXPECT_EQ(0x03981204u, reinterpret_cast<const uint32_t&>(*buffers[0].pData));

		EXPECT_EQ(test::AsVoidPointer(pEntity.get()), buffers[1].pData);
		ASSERT_EQ(124u, buffers[1].Size);
		EXPECT_EQ(*pEntity, reinterpret_cast<const model::VerifiableEntity&>(*buffers[1].pData));

		ASSERT_EQ(12u, buffers[2].Size);
		EXPECT_EQ_MEMORY(valuesBuffer.data(), buffers[2].pData, buffers[2].Size);

		ASSERT_EQ(4u, buffers[3].Size);
		EXPECT_EQ(0x11111111u, reinterpret_cast<const uint32_t&>(*buffers[3].pData));

		ASSERT_EQ(8u, buffers[4].Size);
		EXPECT_EQ_MEMORY(rangeBuffer.data(), buffers[4].pData, buffers[4].Size);

		ASSERT_EQ(4u, buffers[5].Size);
		EXPECT_EQ(0x00003322u, reinterpret_cast<const uint32_t&>(*buffers[5].pData));
	}

	// endregion
}}
