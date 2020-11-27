/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "MosaicRestrictionEntryMapperTestUtils.h"
#include "mongo/src/mappers/MapperInclude.h"
#include "plugins/txes/restriction_mosaic/src/state/MosaicRestrictionEntry.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	state::MosaicAddressRestriction MosaicAddressRestrictionTestTraits::CreateRestriction(size_t numValues) {
		auto mosaicId = GenerateRandomValue<MosaicId>();
		auto address = GenerateRandomByteArray<Address>();
		state::MosaicAddressRestriction addressRestriction(mosaicId, address);
		for (auto i = 0u; i < numValues; ++i)
			addressRestriction.set(i, Random());

		return addressRestriction;
	}

	void MosaicAddressRestrictionTestTraits::AssertEqualRestriction(
			const state::MosaicRestrictionEntry& restrictionEntry,
			const bsoncxx::document::view& dbRestrictionEntry) {
		EXPECT_EQ(6u, GetFieldCount(dbRestrictionEntry));
		EXPECT_EQ(1u, GetUint32(dbRestrictionEntry, "version"));

		const auto& restriction = restrictionEntry.asAddressRestriction();
		auto compositeHash = restrictionEntry.uniqueKey();
		EXPECT_EQ(compositeHash, GetHashValue(dbRestrictionEntry, "compositeHash"));

		auto dbEntryType = static_cast<state::MosaicRestrictionEntry::EntryType>(GetUint8(dbRestrictionEntry, "entryType"));
		EXPECT_EQ(state::MosaicRestrictionEntry::EntryType::Address, dbEntryType);
		EXPECT_EQ(restriction.mosaicId(), MosaicId(GetUint64(dbRestrictionEntry, "mosaicId")));
		EXPECT_EQ(restriction.address(), GetAddressValue(dbRestrictionEntry, "targetAddress"));

		auto dbRestrictions = dbRestrictionEntry["restrictions"].get_array().value;
		auto keys = restriction.keys();
		ASSERT_EQ(keys.size(), GetFieldCount(dbRestrictions));

		for (const auto& dbRestriction : dbRestrictions) {
			auto dbRestrictionView = dbRestriction.get_document().view();
			auto dbKey = GetUint64(dbRestrictionView, "key");
			EXPECT_CONTAINS(keys, dbKey);

			EXPECT_EQ(restriction.get(dbKey), GetUint64(dbRestrictionView, "value"));
			keys.erase(dbKey);
		}
	}

	state::MosaicGlobalRestriction MosaicGlobalRestrictionTestTraits::CreateRestriction(size_t numValues) {
		state::MosaicGlobalRestriction globalRestriction(GenerateRandomValue<MosaicId>());
		for (auto i = 0u; i < numValues; ++i) {
			state::MosaicGlobalRestriction::RestrictionRule rule;
			rule.ReferenceMosaicId = GenerateRandomValue<MosaicId>();
			rule.RestrictionValue = Random();
			rule.RestrictionType = static_cast<model::MosaicRestrictionType>(2 * i + 1);
			globalRestriction.set(i, rule);
		}

		return globalRestriction;
	}

	void MosaicGlobalRestrictionTestTraits::AssertEqualRestriction(
			const state::MosaicRestrictionEntry& restrictionEntry,
			const bsoncxx::document::view& dbRestrictionEntry) {
		EXPECT_EQ(5u, GetFieldCount(dbRestrictionEntry));
		EXPECT_EQ(1u, GetUint32(dbRestrictionEntry, "version"));

		const auto& restriction = restrictionEntry.asGlobalRestriction();
		auto compositeHash = restrictionEntry.uniqueKey();
		EXPECT_EQ(compositeHash, GetHashValue(dbRestrictionEntry, "compositeHash"));

		auto dbEntryType = static_cast<state::MosaicRestrictionEntry::EntryType>(GetUint8(dbRestrictionEntry, "entryType"));
		EXPECT_EQ(state::MosaicRestrictionEntry::EntryType::Global, dbEntryType);
		EXPECT_EQ(restriction.mosaicId(), MosaicId(GetUint64(dbRestrictionEntry, "mosaicId")));

		auto dbRestrictions = dbRestrictionEntry["restrictions"].get_array().value;
		auto keys = restriction.keys();
		ASSERT_EQ(keys.size(), GetFieldCount(dbRestrictions));

		for (const auto& dbRestriction : dbRestrictions) {
			auto dbRestrictionView = dbRestriction.get_document().view();
			auto dbKey = GetUint64(dbRestrictionView, "key");
			EXPECT_CONTAINS(keys, dbKey);

			auto dbRuleView = dbRestrictionView["restriction"].get_document().view();
			state::MosaicGlobalRestriction::RestrictionRule rule;
			bool isSet = restriction.tryGet(dbKey, rule);
			ASSERT_TRUE(isSet);

			auto restrictionType = static_cast<model::MosaicRestrictionType>(GetUint8(dbRuleView, "restrictionType"));
			EXPECT_EQ(rule.ReferenceMosaicId, MosaicId(GetUint64(dbRuleView, "referenceMosaicId")));
			EXPECT_EQ(rule.RestrictionValue, GetUint64(dbRuleView, "restrictionValue"));
			EXPECT_EQ(rule.RestrictionType, restrictionType);
		}
	}
}}
