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

#include "MosaicMapperTestUtils.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/mosaic/src/state/MosaicEntry.h"
#include "catapult/utils/Casting.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		void AssertMosaicProperties(const model::MosaicProperties& properties, const bsoncxx::document::view& dbProperties) {
			ASSERT_EQ(properties.size(), test::GetFieldCount(dbProperties));
			auto dbIter = dbProperties.cbegin();
			for (const auto& property : properties) {
				EXPECT_EQ(property.Id, static_cast<model::MosaicPropertyId>(GetUint8(*dbIter, "id")));
				EXPECT_EQ(property.Value, GetUint64(*dbIter, "value"));
				++dbIter;
			}
		}
	}

	void AssertEqualMosaicData(const state::MosaicEntry& entry, const bsoncxx::document::view& dbMosaic) {
		EXPECT_EQ(entry.mosaicId(), MosaicId(GetUint64(dbMosaic, "mosaicId")));
		EXPECT_EQ(entry.supply(), Amount(GetUint64(dbMosaic, "supply")));

		const auto& definition = entry.definition();
		EXPECT_EQ(definition.height(), Height(GetUint64(dbMosaic, "height")));
		EXPECT_EQ(definition.owner(), GetKeyValue(dbMosaic, "owner"));
		EXPECT_EQ(definition.revision(), GetUint32(dbMosaic, "revision"));

		auto dbProperties = dbMosaic["properties"].get_array().value;
		const auto& properties = definition.properties();

		// two required and one optional property
		EXPECT_EQ(3u, test::GetFieldCount(dbProperties));
		AssertMosaicProperties(properties, dbProperties);
	}
}}
