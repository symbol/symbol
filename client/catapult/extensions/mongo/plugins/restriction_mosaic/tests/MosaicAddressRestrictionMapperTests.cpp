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

#include "src/MosaicAddressRestrictionMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/restriction_mosaic/src/model/MosaicAddressRestrictionTransaction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicAddressRestrictionMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(MosaicAddressRestriction,)
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Mosaic_Address_Restriction)

	// region streamTransaction

	PLUGIN_TEST(CanMapMosaicAddressRestrictionTransaction) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.MosaicId = test::GenerateRandomValue<UnresolvedMosaicId>();
		transaction.RestrictionKey = test::Random();
		transaction.TargetAddress = test::GenerateRandomByteArray<UnresolvedAddress>();
		transaction.PreviousRestrictionValue = test::Random();
		transaction.NewRestrictionValue = test::Random();

		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, transaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(5u, test::GetFieldCount(view));
		EXPECT_EQ(transaction.MosaicId, UnresolvedMosaicId(test::GetUint64(view, "mosaicId")));
		EXPECT_EQ(transaction.RestrictionKey, test::GetUint64(view, "restrictionKey"));
		EXPECT_EQ(transaction.TargetAddress, test::GetUnresolvedAddressValue(view, "targetAddress"));
		EXPECT_EQ(transaction.PreviousRestrictionValue, test::GetUint64(view, "previousRestrictionValue"));
		EXPECT_EQ(transaction.NewRestrictionValue, test::GetUint64(view, "newRestrictionValue"));
	}

	// endregion
}}}
