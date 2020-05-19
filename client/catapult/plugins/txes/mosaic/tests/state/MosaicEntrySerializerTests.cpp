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
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/core/SerializerTestUtils.h"
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
			Address Owner;
			uint32_t Revision;
			model::MosaicFlags Flags;
			uint8_t Divisibility;
			BlockDuration Duration;
		};
	}

#pragma pack(pop)

	// endregion

	// region Save

	namespace {
		model::MosaicProperties CreateMosaicProperties(uint64_t seed) {
			return model::MosaicProperties(static_cast<model::MosaicFlags>(seed), static_cast<uint8_t>(seed + 1), BlockDuration(seed + 4));
		}

		void AssertEntryHeader(
				const std::vector<uint8_t>& buffer,
				MosaicId mosaicId,
				Amount supply,
				Height height,
				const Address& owner,
				uint32_t revision,
				uint64_t propertiesSeed) {
			auto message = "entry header at 0";
			const auto& entryHeader = reinterpret_cast<const MosaicEntryHeader&>(buffer[0]);

			// - id and supply
			EXPECT_EQ(mosaicId, entryHeader.MosaicId) << message;
			EXPECT_EQ(supply, entryHeader.Supply) << message;

			// - definition
			EXPECT_EQ(height, entryHeader.Height) << message;
			EXPECT_EQ(owner, entryHeader.Owner) << message;
			EXPECT_EQ(revision, entryHeader.Revision) << message;

			// - properties
			EXPECT_EQ(static_cast<model::MosaicFlags>(propertiesSeed), entryHeader.Flags);
			EXPECT_EQ(static_cast<uint8_t>(propertiesSeed + 1), entryHeader.Divisibility);
			EXPECT_EQ(BlockDuration(propertiesSeed + 4), entryHeader.Duration);
		}
	}

	TEST(TEST_CLASS, CanSaveEntry) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);

		auto definition = MosaicDefinition(Height(888), test::CreateRandomOwner(), 5, CreateMosaicProperties(17));
		auto entry = MosaicEntry(MosaicId(123), definition);
		entry.increaseSupply(Amount(111));

		// Act:
		MosaicEntrySerializer::Save(entry, stream);

		// Assert:
		ASSERT_EQ(sizeof(MosaicEntryHeader), buffer.size());
		AssertEntryHeader(buffer, MosaicId(123), Amount(111), Height(888), definition.ownerAddress(), 5, 17);
	}

	// endregion

	// region Roundtrip

	TEST(TEST_CLASS, CanRoundtripEntry) {
		// Arrange:
		auto definition = MosaicDefinition(Height(888), test::CreateRandomOwner(), 5, CreateMosaicProperties(17));
		auto originalEntry = MosaicEntry(MosaicId(123), definition);
		originalEntry.increaseSupply(Amount(111));

		// Act:
		auto result = test::RunRoundtripBufferTest<MosaicEntrySerializer>(originalEntry);

		// Assert:
		test::AssertEqual(originalEntry, result);
	}

	// endregion
}}
