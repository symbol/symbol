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

#include "src/MosaicGlobalRestrictionMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/restriction_mosaic/src/model/MosaicGlobalRestrictionTransaction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicGlobalRestrictionMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(MosaicGlobalRestriction,)
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Mosaic_Global_Restriction)

	// region streamTransaction

	PLUGIN_TEST(CanMapMosaicGlobalRestrictionTransaction) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.MosaicId = test::GenerateRandomValue<UnresolvedMosaicId>();
		transaction.ReferenceMosaicId = test::GenerateRandomValue<UnresolvedMosaicId>();
		transaction.RestrictionKey = test::Random();
		transaction.PreviousRestrictionValue = test::Random();
		transaction.PreviousRestrictionType = static_cast<model::MosaicRestrictionType>(test::RandomByte());
		transaction.NewRestrictionValue = test::Random();
		transaction.NewRestrictionType = static_cast<model::MosaicRestrictionType>(test::RandomByte());

		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, transaction);
		auto view = builder.view();

		// Assert:
		auto previousRestrictionType = static_cast<model::MosaicRestrictionType>(test::GetUint8(view, "previousRestrictionType"));
		auto newRestrictionType = static_cast<model::MosaicRestrictionType>(test::GetUint8(view, "newRestrictionType"));
		EXPECT_EQ(7u, test::GetFieldCount(view));
		EXPECT_EQ(transaction.MosaicId, UnresolvedMosaicId(test::GetUint64(view, "mosaicId")));
		EXPECT_EQ(transaction.ReferenceMosaicId, UnresolvedMosaicId(test::GetUint64(view, "referenceMosaicId")));
		EXPECT_EQ(transaction.RestrictionKey, test::GetUint64(view, "restrictionKey"));
		EXPECT_EQ(transaction.PreviousRestrictionValue, test::GetUint64(view, "previousRestrictionValue"));
		EXPECT_EQ(transaction.PreviousRestrictionType, previousRestrictionType);
		EXPECT_EQ(transaction.NewRestrictionValue, test::GetUint64(view, "newRestrictionValue"));
		EXPECT_EQ(transaction.NewRestrictionType, newRestrictionType);
	}

	// endregion
}}}
