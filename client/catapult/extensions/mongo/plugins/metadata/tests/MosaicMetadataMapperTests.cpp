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

#include "src/MosaicMetadataMapper.h"
#include "plugins/txes/metadata/src/model/MosaicMetadataTransaction.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "tests/test/MetadataMapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicMetadataMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(MosaicMetadata,)
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Mosaic_Metadata)

	// region streamTransaction

	PLUGIN_TEST(CanMapMosaicMetadataTransaction_ZeroValueSize) {
		test::AssertCanMapTransaction<TTraits, test::MosaicMetadataTestTraits>(0);
	}

	PLUGIN_TEST(CanMapMosaicMetadataTransaction_NonzeroValueSize) {
		for (auto valueSize : { 1u, 8u, 123u, 257u })
			test::AssertCanMapTransaction<TTraits, test::MosaicMetadataTestTraits>(static_cast<uint16_t>(valueSize));
	}

	// endregion
}}}
