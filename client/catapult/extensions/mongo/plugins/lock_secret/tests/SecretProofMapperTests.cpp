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

#include "src/SecretProofMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock_secret/src/model/SecretProofTransaction.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "plugins/txes/lock_secret/tests/test/SecretLockTransactionUtils.h"
#include "tests/test/SecretLockMapperTestUtils.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS SecretProofMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(SecretProof,)
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Secret_Proof)

	// region streamTransaction

	PLUGIN_TEST(CanMapSecretProofTransactionWithProof) {
		// Arrange:
		auto pTransaction = test::CreateRandomSecretProofTransaction<TTraits>(123);
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(4u, test::GetFieldCount(view));
		EXPECT_EQ(utils::to_underlying_type(pTransaction->HashAlgorithm), test::GetUint8(view, "hashAlgorithm"));
		EXPECT_EQ(pTransaction->Secret, test::GetHashValue(view, "secret"));
		EXPECT_EQ(pTransaction->RecipientAddress, test::GetUnresolvedAddressValue(view, "recipientAddress"));

		const auto* pProof = pTransaction->ProofPtr();
		const auto* pDbProof = test::GetBinary(view, "proof");
		EXPECT_EQ_MEMORY(pProof, pDbProof, pTransaction->ProofSize);
	}

	// endregion
}}}
