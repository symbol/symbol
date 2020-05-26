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

#include "MetadataMapperTestUtils.h"
#include "plugins/txes/metadata/src/state/MetadataEntry.h"
#include "mongo/tests/test/MapperTestUtils.h"

namespace catapult { namespace test {

	void AssertEqualMetadataEntry(const state::MetadataEntry& metadataEntry, const bsoncxx::document::view& dbMetadataEntry) {
		const auto& key = metadataEntry.key();
		const auto& value = metadataEntry.value();
		EXPECT_EQ(value.empty() ? 7u : 8u, GetFieldCount(dbMetadataEntry));

		auto compositeHash = metadataEntry.key().uniqueKey();
		EXPECT_EQ(compositeHash, GetHashValue(dbMetadataEntry, "compositeHash"));

		EXPECT_EQ(key.sourceAddress(), GetAddressValue(dbMetadataEntry, "sourceAddress"));
		EXPECT_EQ(key.targetAddress(), GetAddressValue(dbMetadataEntry, "targetAddress"));
		EXPECT_EQ(key.scopedMetadataKey(), GetUint64(dbMetadataEntry, "scopedMetadataKey"));
		EXPECT_EQ(key.targetId(), GetUint64(dbMetadataEntry, "targetId"));
		EXPECT_EQ(key.metadataType(), static_cast<model::MetadataType>(GetUint8(dbMetadataEntry, "metadataType")));

		EXPECT_EQ(value.size(), GetUint32(dbMetadataEntry, "valueSize"));
		if (!value.empty()) {
			auto dbValue = dbMetadataEntry["value"].get_binary();
			ASSERT_EQ(value.size(), dbValue.size);
			EXPECT_EQ_MEMORY(value.data(), dbValue.bytes, value.size());
		}
	}
}}
