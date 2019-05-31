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

#include "src/AccountLinkMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/account_link/src/model/AccountLinkTransaction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS AccountLinkMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(AccountLink,)
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Account_Link)

	// region streamTransaction

	PLUGIN_TEST(CanMapAccountLinkTransaction) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.LinkAction = model::AccountLinkAction::Unlink;
		test::FillWithRandomData(transaction.RemoteAccountKey);

		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, transaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(2u, test::GetFieldCount(view));
		EXPECT_EQ(model::AccountLinkAction::Unlink, static_cast<model::AccountLinkAction>(test::GetUint32(view, "action")));
		EXPECT_EQ(transaction.RemoteAccountKey, test::GetKeyValue(view, "remoteAccountKey"));
	}

	// endregion
}}}
