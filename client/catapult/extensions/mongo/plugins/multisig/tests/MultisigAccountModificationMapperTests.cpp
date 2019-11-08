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

#include "src/MultisigAccountModificationMapper.h"
#include "sdk/src/builders/MultisigAccountModificationBuilder.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MultisigAccountModificationMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(MultisigAccountModification)

		auto CreateMultisigAccountModificationTransactionBuilder(
				const Key& signer,
				int8_t minRemovalDelta,
				int8_t minApprovalDelta,
				uint8_t numKeyAdditions,
				uint8_t numKeyDeletions) {
			builders::MultisigAccountModificationBuilder builder(model::NetworkIdentifier::Mijin_Test, signer);
			builder.setMinRemovalDelta(minRemovalDelta);
			builder.setMinApprovalDelta(minApprovalDelta);

			for (auto i = 0u; i < numKeyAdditions; ++i)
				builder.addPublicKeyAddition(test::GenerateRandomByteArray<Key>());

			for (auto i = 0u; i < numKeyDeletions; ++i)
				builder.addPublicKeyDeletion(test::GenerateRandomByteArray<Key>());

			return builder;
		}

		void AssertEqualKeys(const bsoncxx::document::view& dbTransaction, const std::string& name, const Key* pKeys, uint8_t numKeys) {
			auto dbKeys = dbTransaction[name].get_array().value;
			ASSERT_EQ(numKeys, test::GetFieldCount(dbKeys));

			auto dbKeysIter = dbKeys.cbegin();
			for (auto i = 0u; i < numKeys; ++i, ++dbKeysIter) {
				Key key;
				mongo::mappers::DbBinaryToModelArray(key, dbKeysIter->get_binary());
				EXPECT_EQ(pKeys[i], key) << name << " at " << i;
			}
		}

		template<typename TTransaction>
		void AssertEqualNonInheritedTransferData(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
			EXPECT_EQ(transaction.MinRemovalDelta, test::GetInt32(dbTransaction, "minRemovalDelta"));
			EXPECT_EQ(transaction.MinApprovalDelta, test::GetInt32(dbTransaction, "minApprovalDelta"));

			AssertEqualKeys(dbTransaction, "publicKeyAdditions", transaction.PublicKeyAdditionsPtr(), transaction.PublicKeyAdditionsCount);
			AssertEqualKeys(dbTransaction, "publicKeyDeletions", transaction.PublicKeyDeletionsPtr(), transaction.PublicKeyDeletionsCount);
		}

		template<typename TTraits>
		void AssertCanMapMultisigAccountModificationTransaction(
				int8_t minRemovalDelta,
				int8_t minApprovalDelta,
				uint8_t numKeyAdditions,
				uint8_t numKeyDeletions) {
			// Arrange:
			auto signer = test::GenerateRandomByteArray<Key>();
			auto pBuilder = CreateMultisigAccountModificationTransactionBuilder(
					signer,
					minRemovalDelta,
					minApprovalDelta,
					numKeyAdditions,
					numKeyDeletions);
			auto pTransaction = TTraits::Adapt(pBuilder);
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(4u, test::GetFieldCount(view));
			AssertEqualNonInheritedTransferData(*pTransaction, view);
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Multisig_Account_Modification)

	// region streamTransaction

	PLUGIN_TEST(CanMapModifiyMultisigAccountTransactionWithoutModification) {
		AssertCanMapMultisigAccountModificationTransaction<TTraits>(3, 5, 0, 0);
	}

	PLUGIN_TEST(CanMapModifiyMultisigAccountTransactionWithSingleAddition) {
		AssertCanMapMultisigAccountModificationTransaction<TTraits>(3, 5, 1, 0);
	}

	PLUGIN_TEST(CanMapModifiyMultisigAccountTransactionWithSingleDeletion) {
		AssertCanMapMultisigAccountModificationTransaction<TTraits>(3, 5, 0, 1);
	}

	PLUGIN_TEST(CanMapModifiyMultisigAccountTransactionWithMultipleAdditionsAndDeletions) {
		AssertCanMapMultisigAccountModificationTransaction<TTraits>(3, 5, 4, 2);
	}

	// endregion
}}}
