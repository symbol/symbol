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

#include "src/state/MosaicEntrySerializer.h"
#include "src/state/MosaicLevy.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicEntrySerializerTests

	// region headers

#pragma pack(push, 1)

	namespace {
		struct MosaicEntryHeader {
			catapult::MosaicId MosaicId;
			Amount Supply;
			catapult::Height Height;
			Key Owner;
			uint32_t Revision;
			std::array<uint64_t, model::Num_Mosaic_Properties> PropertyValues;
		};
	}

#pragma pack(pop)

	// endregion

	// region Save

	namespace {
		std::unique_ptr<MosaicLevy> CreateMosaicLevy() {
			return std::make_unique<MosaicLevy>(
					MosaicId(9988),
					test::GenerateRandomData<Address_Decoded_Size>(),
					std::vector<MosaicLevyRule>());
		}

		model::MosaicProperties CreateMosaicProperties(uint64_t seed) {
			auto values = model::MosaicProperties::PropertyValuesContainer();

			uint8_t i = 0;
			for (auto& value : values) {
				value = i * i + seed;
				++i;
			}

			return model::MosaicProperties::FromValues(values);
		}

		void AssertEntryHeader(
				const std::vector<uint8_t>& buffer,
				MosaicId mosaicId,
				Amount supply,
				Height height,
				const Key& owner,
				uint32_t revision,
				uint64_t propertiesSeed) {
			auto message = "entry header at 0";
			const auto& entryHeader = reinterpret_cast<const MosaicEntryHeader&>(*buffer.data());

			// - id and supply
			EXPECT_EQ(mosaicId, entryHeader.MosaicId) << message;
			EXPECT_EQ(supply, entryHeader.Supply) << message;

			// - definition
			EXPECT_EQ(height, entryHeader.Height) << message;
			EXPECT_EQ(owner, entryHeader.Owner) << message;
			EXPECT_EQ(revision, entryHeader.Revision) << message;
			for (auto i = 0u; i < model::Num_Mosaic_Properties; ++i)
				EXPECT_EQ(i * i + propertiesSeed, entryHeader.PropertyValues[i]) << message << " property " << i;
		}
	}

	TEST(TEST_CLASS, CannotSaveEntryWithMosaicLevy) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		// - add a levy
		auto definition = MosaicDefinition(Height(888), test::GenerateRandomData<Key_Size>(), 1, CreateMosaicProperties(17));
		auto entry = MosaicEntry(MosaicId(123), definition);
		entry.increaseSupply(Amount(111));
		entry.setLevy(CreateMosaicLevy());

		// Act + Assert:
		EXPECT_THROW(MosaicEntrySerializer::Save(entry, stream), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanSaveEntryWithoutMosaicLevy) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		auto definition = MosaicDefinition(Height(888), test::GenerateRandomData<Key_Size>(), 5, CreateMosaicProperties(17));
		auto entry = MosaicEntry(MosaicId(123), definition);
		entry.increaseSupply(Amount(111));

		// Act:
		MosaicEntrySerializer::Save(entry, stream);

		// Assert:
		ASSERT_EQ(sizeof(MosaicEntryHeader), buffer.size());
		AssertEntryHeader(buffer, MosaicId(123), Amount(111), Height(888), definition.owner(), 5, 17);
	}

	// endregion

	// region Load

	namespace {
		void AssertMosaicEntry(
				const state::MosaicEntry& entry,
				MosaicId mosaicId,
				Amount supply,
				Height height,
				const Key& owner,
				uint32_t revision,
				const decltype(MosaicEntryHeader::PropertyValues)& propertyValues) {
			auto message = "entry " + std::to_string(entry.mosaicId().unwrap());

			// - entry
			EXPECT_EQ(mosaicId, entry.mosaicId()) << message;
			EXPECT_EQ(supply, entry.supply()) << message;

			// - definition
			const auto& definition = entry.definition();
			EXPECT_EQ(height, definition.height()) << message;
			EXPECT_EQ(owner, definition.owner()) << message;
			EXPECT_EQ(revision, definition.revision()) << message;

			uint8_t i = 0;
			for (const auto& property : definition.properties()) {
				EXPECT_EQ(static_cast<model::MosaicPropertyId>(i), property.Id) << message << " property " << i;
				EXPECT_EQ(propertyValues[i], property.Value) << message << " property " << i;
				++i;
			}
		}
	}

	TEST(TEST_CLASS, CanLoadEntryWithoutMosaicLevy) {
		// Arrange:
		auto owner = test::GenerateRandomData<Key_Size>();
		std::vector<uint8_t> buffer(sizeof(MosaicEntryHeader));
		reinterpret_cast<MosaicEntryHeader&>(*buffer.data()) = { MosaicId(123), Amount(786), Height(222), owner, 5, { { 9, 8, 7 } } };
		mocks::MockMemoryStream stream("", buffer);

		// Act:
		auto entry = MosaicEntrySerializer::Load(stream);

		// Assert:
		AssertMosaicEntry(entry, MosaicId(123), Amount(786), Height(222), owner, 5, { { 9, 8, 7 } });
	}

	// endregion
}}
