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

#include "src/NamespaceRegistrationMapper.h"
#include "sdk/src/builders/NamespaceRegistrationBuilder.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS NamespaceRegistrationMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(NamespaceRegistration)

		auto CreateNamespaceRegistrationTransactionBuilder(
				const Key& signer,
				model::NamespaceRegistrationType registrationType,
				const std::string& namespaceName) {
			builders::NamespaceRegistrationBuilder builder(model::NetworkIdentifier::Private_Test, signer);
			builder.setName({ reinterpret_cast<const uint8_t*>(namespaceName.data()), namespaceName.size() });

			if (model::NamespaceRegistrationType::Root == registrationType)
				builder.setDuration(test::GenerateRandomValue<BlockDuration>());
			else
				builder.setParentId(test::GenerateRandomValue<NamespaceId>());

			return builder;
		}

		void AssertSharedNamespaceRegistrationData(
				model::NamespaceRegistrationType registrationType,
				NamespaceId id,
				const std::string& name,
				const bsoncxx::document::view& dbTransaction) {
			// Assert:
			EXPECT_EQ(registrationType, static_cast<model::NamespaceRegistrationType>(test::GetUint32(dbTransaction, "registrationType")));
			EXPECT_EQ(id, NamespaceId(test::GetUint64(dbTransaction, "id")));

			auto dbName = dbTransaction["name"].get_binary();
			EXPECT_EQ(name.size(), dbName.size);
			EXPECT_EQ_MEMORY(name.data(), dbName.bytes, name.size());
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , model::Entity_Type_Namespace_Registration)

	// region streamTransaction

	PLUGIN_TEST(CannotMapNamespaceRegistrationTransactionWithoutName) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.Size = sizeof(typename TTraits::TransactionType);
		transaction.Type = model::Entity_Type_Namespace_Registration;
		transaction.RegistrationType = model::NamespaceRegistrationType::Root;
		transaction.NameSize = 0;

		auto pPlugin = TTraits::CreatePlugin();

		// Act + Assert:
		mappers::bson_stream::document builder;
		EXPECT_THROW(pPlugin->streamTransaction(builder, transaction), catapult_runtime_error);
	}

	PLUGIN_TEST(CanMapRootNamespaceRegistrationTransactionWithName) {
		// Arrange:
		std::string namespaceName("jabo38");
		auto signer = test::GenerateRandomByteArray<Key>();
		auto pTransaction = TTraits::Adapt(CreateNamespaceRegistrationTransactionBuilder(
				signer,
				model::NamespaceRegistrationType::Root,
				namespaceName));
		auto namespaceId = pTransaction->Id;
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(4u, test::GetFieldCount(view));
		AssertSharedNamespaceRegistrationData(pTransaction->RegistrationType, namespaceId, namespaceName, view);
		EXPECT_EQ(pTransaction->Duration, BlockDuration(test::GetUint64(view, "duration")));
	}

	PLUGIN_TEST(CanMapChildNamespaceRegistrationTransactionWithName) {
		// Arrange:
		std::string namespaceName("jabo38");
		auto signer = test::GenerateRandomByteArray<Key>();
		auto pTransaction = TTraits::Adapt(CreateNamespaceRegistrationTransactionBuilder(
				signer,
				model::NamespaceRegistrationType::Child,
				namespaceName));
		auto namespaceId = pTransaction->Id;
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(4u, test::GetFieldCount(view));
		AssertSharedNamespaceRegistrationData(pTransaction->RegistrationType, namespaceId, namespaceName, view);
		EXPECT_EQ(pTransaction->ParentId, NamespaceId(test::GetUint64(view, "parentId")));
	}

	// endregion
}}}
