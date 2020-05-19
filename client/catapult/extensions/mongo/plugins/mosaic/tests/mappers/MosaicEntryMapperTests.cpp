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

#include "src/mappers/MosaicEntryMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/mosaic/src/state/MosaicEntry.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "plugins/txes/mosaic/tests/test/MosaicTestUtils.h"
#include "tests/test/MosaicMapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicEntryMapperTests

	// region ToDbModel

	namespace {
		state::MosaicEntry CreateMosaicEntry() {
			auto owner = test::CreateRandomOwner();
			return test::CreateMosaicEntry(MosaicId(345), Height(123), owner, Amount(456), BlockDuration(12345));
		}
	}

	TEST(TEST_CLASS, CanMapMosaicEntry_ModelToDbModel) {
		// Arrange:
		auto entry = CreateMosaicEntry();

		// Act:
		auto document = ToDbModel(entry);
		auto documentView = document.view();

		// Assert:
		EXPECT_EQ(1u, test::GetFieldCount(documentView));

		auto mosaicView = documentView["mosaic"].get_document().view();
		EXPECT_EQ(8u, test::GetFieldCount(mosaicView));
		test::AssertEqualMosaicData(entry, mosaicView);
	}

	// endregion
}}}
