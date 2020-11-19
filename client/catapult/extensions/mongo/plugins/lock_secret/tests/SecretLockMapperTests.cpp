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

#include "src/SecretLockMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock_secret/src/model/SecretLockTransaction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "plugins/txes/lock_shared/tests/test/LockTransactionUtils.h"
#include "tests/test/SecretLockMapperTestUtils.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS SecretLockMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(SecretLock,)

		template<typename TTransaction>
		void AssertSecretLockTransaction(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
			EXPECT_EQ(utils::to_underlying_type(transaction.HashAlgorithm), test::GetUint32(dbTransaction, "hashAlgorithm"));
			EXPECT_EQ(transaction.Secret, test::GetBinaryArray<Hash256::Size>(dbTransaction, "secret"));
			EXPECT_EQ(transaction.RecipientAddress, test::GetUnresolvedAddressValue(dbTransaction, "recipientAddress"));
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Secret_Lock)

	// region streamTransaction

	PLUGIN_TEST(CanMapSecretLockTransaction) {
		// Arrange:
		auto pTransaction = test::CreateRandomLockTransaction<TTraits>();
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(6u, test::GetFieldCount(view));
		test::AssertEqualNonInheritedLockTransactionData(*pTransaction, view);
		AssertSecretLockTransaction(*pTransaction, view);
	}

	// endregion
}}}
