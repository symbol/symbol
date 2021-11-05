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

#include "src/MosaicSupplyRevocationMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/mosaic/src/model/MosaicSupplyRevocationTransaction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MosaicSupplyRevocationMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(MosaicSupplyRevocation,)
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Mosaic_Supply_Revocation)

	// region streamTransaction

	PLUGIN_TEST(CanMapSupplyRevocationTransaction) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.SourceAddress = test::GenerateRandomByteArray<UnresolvedAddress>();
		transaction.Mosaic.MosaicId = UnresolvedMosaicId(998877);
		transaction.Mosaic.Amount = Amount(12349876);

		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, transaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(3u, test::GetFieldCount(view));
		EXPECT_EQ(transaction.SourceAddress, test::GetUnresolvedAddressValue(view, "sourceAddress"));
		EXPECT_EQ(UnresolvedMosaicId(998877), UnresolvedMosaicId(test::GetUint64(view, "mosaicId")));
		EXPECT_EQ(Amount(12349876), Amount(test::GetUint64(view, "amount")));
	}

	// endregion
}}}
