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

#include "src/MosaicExpiryReceiptMapper.h"
#include "mongo/src/MongoReceiptPluginFactory.h"
#include "plugins/txes/mosaic/src/model/MosaicReceiptType.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/core/mocks/MockReceipt.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicExpiryReceiptMapperTests

	TEST(TEST_CLASS, CanCreateMosaicExpiryReceiptPlugin) {
		// Act:
		auto pPlugin = CreateMosaicExpiryReceiptMongoPlugin();

		// Assert:
		EXPECT_EQ(model::Receipt_Type_Mosaic_Expired, pPlugin->type());
	}

	TEST(TEST_CLASS, CanStreamMosaicExpiryReceipt) {
		// Arrange:
		auto pPlugin = CreateMosaicExpiryReceiptMongoPlugin();
		bsoncxx::builder::stream::document builder;

		model::ArtifactExpiryReceipt<MosaicId> receipt(model::ReceiptType(), MosaicId(234));

		// Act:
		pPlugin->streamReceipt(builder, receipt);
		auto dbReceipt = builder << bsoncxx::builder::stream::finalize;

		// Assert:
		auto view = dbReceipt.view();
		EXPECT_EQ(1u, test::GetFieldCount(view));

		EXPECT_EQ(receipt.ArtifactId, MosaicId(test::GetUint64(view, "artifactId")));
	}
}}}
