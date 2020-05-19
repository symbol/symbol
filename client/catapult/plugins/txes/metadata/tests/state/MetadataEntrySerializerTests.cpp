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

#include "src/state/MetadataEntrySerializer.h"
#include "tests/test/MetadataTestUtils.h"
#include "tests/test/core/SerializerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MetadataEntrySerializerTests

	namespace {
		// region raw structures

#pragma pack(push, 1)

		struct MetadataEntryHeader {
			Address SourceAddress;
			Address TargetAddress;
			uint64_t ScopedMetadataKey;
			uint64_t TargetId;
			model::MetadataType MetadataType;
			uint16_t ValueSize;
		};

#pragma pack(pop)

		// endregion

		// region test utils

		template<typename TTraits>
		MetadataEntry CreateRandomMetadataEntry(const std::vector<uint8_t>& valueBuffer) {
			auto key = TTraits::CreateKeyWithTargetId(test::GenerateRandomPartialMetadataKey(), test::Random());
			auto entry = MetadataEntry(key);
			entry.value().update(valueBuffer);
			return entry;
		}

		// endregion
	}

	// region traits

	namespace {
		struct AccountTraits {
			static constexpr auto Metadata_Type = model::MetadataType::Account;

			static MetadataKey CreateKeyWithTargetId(const model::PartialMetadataKey& partialKey, uint64_t) {
				return MetadataKey(partialKey);
			}
		};

		struct MosaicTraits {
			static constexpr auto Metadata_Type = model::MetadataType::Mosaic;

			static MetadataKey CreateKeyWithTargetId(const model::PartialMetadataKey& partialKey, uint64_t targetId) {
				return MetadataKey(partialKey, MosaicId(targetId));
			}
		};

		struct NamespaceTraits {
			static constexpr auto Metadata_Type = model::MetadataType::Namespace;

			static MetadataKey CreateKeyWithTargetId(const model::PartialMetadataKey& partialKey, uint64_t targetId) {
				return MetadataKey(partialKey, NamespaceId(targetId));
			}
		};
	}

#define METADATA_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Account) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Namespace) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NamespaceTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region Save

	namespace {
		void AssertEqual(const MetadataEntryHeader& header, const MetadataKey& key) {
			EXPECT_EQ(header.SourceAddress, key.sourceAddress());
			EXPECT_EQ(header.TargetAddress, key.targetAddress());
			EXPECT_EQ(header.ScopedMetadataKey, key.scopedMetadataKey());
			EXPECT_EQ(header.TargetId, key.targetId());
			EXPECT_EQ(header.MetadataType, key.metadataType());
		}
	}

	METADATA_TRAITS_BASED_TEST(CanSaveEntryWithEmptyValue) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		auto entry = CreateRandomMetadataEntry<TTraits>(std::vector<uint8_t>());

		// Act:
		MetadataEntrySerializer::Save(entry, outputStream);

		// Assert:
		ASSERT_EQ(sizeof(MetadataEntryHeader), buffer.size());

		const auto& header = reinterpret_cast<const MetadataEntryHeader&>(buffer[0]);
		EXPECT_EQ(TTraits::Metadata_Type, header.MetadataType);
		AssertEqual(header, entry.key());

		EXPECT_EQ(0u, static_cast<uint16_t>(header.ValueSize)); // not aligned so cannot be passed by reference
	}

	METADATA_TRAITS_BASED_TEST(CanSaveEntryWithValue) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		auto valueBuffer = test::GenerateRandomVector(11);
		auto entry = CreateRandomMetadataEntry<TTraits>(valueBuffer);

		// Act:
		MetadataEntrySerializer::Save(entry, outputStream);

		// Assert:
		ASSERT_EQ(sizeof(MetadataEntryHeader) + 11u, buffer.size());

		const auto& header = reinterpret_cast<const MetadataEntryHeader&>(buffer[0]);
		EXPECT_EQ(TTraits::Metadata_Type, header.MetadataType);
		AssertEqual(header, entry.key());

		ASSERT_EQ(11u, static_cast<uint16_t>(header.ValueSize)); // not aligned so cannot be passed by reference
		EXPECT_EQ_MEMORY(valueBuffer.data(), &buffer[sizeof(MetadataEntryHeader)], valueBuffer.size());
	}

	// endregion

	// region Load (failure)

	METADATA_TRAITS_BASED_TEST(CannotLoadWithInvalidMetadataType) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream outputStream(buffer);

		auto entry = CreateRandomMetadataEntry<TTraits>(std::vector<uint8_t>());

		MetadataEntrySerializer::Save(entry, outputStream);

		// Sanity:
		ASSERT_EQ(sizeof(MetadataEntryHeader), buffer.size());

		// - corrupt the type
		auto& header = reinterpret_cast<MetadataEntryHeader&>(buffer[0]);
		header.MetadataType = static_cast<model::MetadataType>(3);

		// Act + Assert:
		EXPECT_THROW(MetadataEntrySerializer::Load(outputStream), catapult_invalid_argument);
	}

	// endregion

	// region Roundtrip

	namespace {
		template<typename TTraits>
		void AssertCanRoundtripEntry(uint8_t count) {
			// Arrange:
			auto originalEntry = CreateRandomMetadataEntry<TTraits>(test::GenerateRandomVector(count));

			// Act:
			auto result = test::RunRoundtripBufferTest<MetadataEntrySerializer>(originalEntry);

			// Assert:
			test::AssertEqual(originalEntry, result);

			// Sanity:
			EXPECT_EQ(count, result.value().size());
		}
	}

	METADATA_TRAITS_BASED_TEST(CanRoundtripEmptyEntry) {
		AssertCanRoundtripEntry<TTraits>(0);
	}

	METADATA_TRAITS_BASED_TEST(CanRoundtripEntry) {
		AssertCanRoundtripEntry<TTraits>(3);
	}

	// endregion
}}
