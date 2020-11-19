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

#include "src/AccountLinkMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/account_link/src/model/AccountKeyLinkTransaction.h"
#include "plugins/txes/account_link/src/model/NodeKeyLinkTransaction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS AccountLinkMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(AccountKeyLink, Account)
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(NodeKeyLink, Node)
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Account, _Account, model::Entity_Type_Account_Key_Link)
	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Node, _Node, model::Entity_Type_Node_Key_Link)

#undef PLUGIN_TEST

#define PLUGIN_TEST_ENTRY(NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_##NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NAME##RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_##NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NAME##EmbeddedTraits>(); } \

#define PLUGIN_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	PLUGIN_TEST_ENTRY(Account, TEST_NAME) \
	PLUGIN_TEST_ENTRY(Node, TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region streamTransaction

	PLUGIN_TEST(CanMapLinkTransaction) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.LinkAction = model::LinkAction::Unlink;
		test::FillWithRandomData(transaction.LinkedPublicKey);

		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, transaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(2u, test::GetFieldCount(view));
		EXPECT_EQ(model::LinkAction::Unlink, static_cast<model::LinkAction>(test::GetUint32(view, "linkAction")));
		EXPECT_EQ(transaction.LinkedPublicKey, test::GetKeyValue(view, "linkedPublicKey"));
	}

	// endregion
}}}
