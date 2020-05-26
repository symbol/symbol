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
			EXPECT_EQ(properties.flags(), static_cast<model::MosaicFlags>(GetUint8(dbProperties, "flags")));
			EXPECT_EQ(properties.divisibility(), GetUint8(dbProperties, "divisibility"));
			EXPECT_EQ(properties.duration(), BlockDuration(GetUint64(dbProperties, "duration")));
		}
	}

	void AssertEqualMosaicData(const state::MosaicEntry& mosaicEntry, const bsoncxx::document::view& dbMosaicEntry) {
		EXPECT_EQ(mosaicEntry.mosaicId(), MosaicId(GetUint64(dbMosaicEntry, "id")));
		EXPECT_EQ(mosaicEntry.supply(), Amount(GetUint64(dbMosaicEntry, "supply")));

		const auto& definition = mosaicEntry.definition();
		EXPECT_EQ(definition.startHeight(), Height(GetUint64(dbMosaicEntry, "startHeight")));
		EXPECT_EQ(definition.ownerAddress(), GetAddressValue(dbMosaicEntry, "ownerAddress"));
		EXPECT_EQ(definition.revision(), GetUint32(dbMosaicEntry, "revision"));

		AssertMosaicProperties(definition.properties(), dbMosaicEntry);
	}
}}
