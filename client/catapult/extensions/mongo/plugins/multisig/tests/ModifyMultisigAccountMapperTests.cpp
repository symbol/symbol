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

#include "src/ModifyMultisigAccountMapper.h"
#include "sdk/src/builders/ModifyMultisigAccountBuilder.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS ModifyMultisigAccountMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(ModifyMultisigAccount)

		using ModificationType = model::CosignatoryModificationType;

		auto CreateModifyMultisigAccountTransactionBuilder(
				const Key& signer,
				int8_t minRemovalDelta,
				int8_t minApprovalDelta,
				std::initializer_list<model::CosignatoryModification> modifications) {
			builders::ModifyMultisigAccountBuilder builder(model::NetworkIdentifier::Mijin_Test, signer);
			builder.setMinRemovalDelta(minRemovalDelta);
			builder.setMinApprovalDelta(minApprovalDelta);
			for (const auto& modification : modifications)
				builder.addModification(modification);

			return builder;
		}

		template<typename TTransaction>
		void AssertEqualNonInheritedTransferData(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
			EXPECT_EQ(transaction.MinRemovalDelta, test::GetInt32(dbTransaction, "minRemovalDelta"));
			EXPECT_EQ(transaction.MinApprovalDelta, test::GetInt32(dbTransaction, "minApprovalDelta"));

			auto dbModifications = dbTransaction["modifications"].get_array().value;
			ASSERT_EQ(transaction.ModificationsCount, test::GetFieldCount(dbModifications));
			const auto* pModification = transaction.ModificationsPtr();
			auto iter = dbModifications.cbegin();
			for (auto i = 0u; i < transaction.ModificationsCount; ++i) {
				EXPECT_EQ(
						pModification->ModificationType,
						static_cast<ModificationType>(test::GetUint32(iter->get_document().view(), "type")));
				EXPECT_EQ(pModification->CosignatoryPublicKey, test::GetKeyValue(iter->get_document().view(), "cosignatoryPublicKey"));
				++pModification;
				++iter;
			}
		}

		template<typename TTraits>
		void AssertCanMapModifyMultisigAccountTransaction(
				int8_t minRemovalDelta,
				int8_t minApprovalDelta,
				std::initializer_list<model::CosignatoryModification> modifications) {
			// Arrange:
			auto signer = test::GenerateRandomData<Key_Size>();
			auto pBuilder = CreateModifyMultisigAccountTransactionBuilder(signer, minRemovalDelta, minApprovalDelta, modifications);
			auto pTransaction = TTraits::Adapt(pBuilder);
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(3u, test::GetFieldCount(view));
			AssertEqualNonInheritedTransferData(*pTransaction, view);
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, model::Entity_Type_Modify_Multisig_Account)

	// region streamTransaction

	PLUGIN_TEST(CanMapModifiyMultisigAccountTransactionWithoutModification) {
		// Assert:
		AssertCanMapModifyMultisigAccountTransaction<TTraits>(3, 5, {});
	}

	PLUGIN_TEST(CanMapModifiyMultisigAccountTransactionWithSingleModification) {
		// Assert:
		AssertCanMapModifyMultisigAccountTransaction<TTraits>(3, 5, { { ModificationType::Add, test::GenerateRandomData<Key_Size>() } });
	}

	PLUGIN_TEST(CanMapModifiyMultisigAccountTransactionWithMultipleModification) {
		// Assert:
		AssertCanMapModifyMultisigAccountTransaction<TTraits>(3, 5, {
			{ ModificationType::Add, test::GenerateRandomData<Key_Size>() },
			{ ModificationType::Del, test::GenerateRandomData<Key_Size>() },
			{ ModificationType::Add, test::GenerateRandomData<Key_Size>() },
		});
	}

	// endregion
}}}
