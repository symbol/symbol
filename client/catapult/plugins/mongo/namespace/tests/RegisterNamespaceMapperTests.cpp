#include "src/RegisterNamespaceMapper.h"
#include "sdk/src/builders/RegisterNamespaceBuilder.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/test/mongo/MongoTransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

#define TEST_CLASS RegisterNamespaceMapperTests

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(RegisterNamespace);

		auto CreateRegisterNamespaceTransaction(model::NamespaceType namespaceType, const std::string& namespaceName) {
			auto signer = test::GenerateKeyPair();
			builders::RegisterNamespaceBuilder builder(model::NetworkIdentifier::Mijin_Test, signer.publicKey(), namespaceName);

			if (model::NamespaceType::Root == namespaceType)
				builder.setDuration(test::GenerateRandomValue<ArtifactDuration>());
			else
				builder.setParentId(test::GenerateRandomValue<NamespaceId>());

			return builder.build();
		}

		void AssertSharedRegisterNamespaceData(
				model::NamespaceType namespaceType,
				NamespaceId parentId,
				NamespaceId id,
				const std::string& namespaceName,
				const bsoncxx::document::view& dbTransaction) {
			// Assert:
			EXPECT_EQ(namespaceType, static_cast<model::NamespaceType>(test::GetUint32(dbTransaction, "namespaceType")));
			EXPECT_EQ(parentId.unwrap(), test::GetUint64(dbTransaction, "parentId"));
			EXPECT_EQ(id.unwrap(), test::GetUint64(dbTransaction, "namespaceId"));

			auto dbName = dbTransaction["name"].get_binary();
			EXPECT_EQ(namespaceName.size(), dbName.size);
			EXPECT_EQ(
					test::ToHexString(reinterpret_cast<const uint8_t*>(namespaceName.data()), namespaceName.size()),
					test::ToHexString(dbName.bytes, dbName.size));
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, model::EntityType::Register_Namespace)

	// region streamTransaction

	PLUGIN_TEST(CannotMapRegisterNamespaceTransactionWithoutName) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.Size = sizeof(typename TTraits::TransactionType);
		transaction.Type = model::EntityType::Register_Namespace;
		transaction.NamespaceType = model::NamespaceType::Root;
		transaction.NamespaceNameSize = 0;

		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		EXPECT_THROW(pPlugin->streamTransaction(builder, transaction), catapult_runtime_error);
	}

	PLUGIN_TEST(CanMapRootRegisterNamespaceTransactionWithName) {
		// Arrange:
		std::string namespaceName("jabo38");
		auto pTransaction = TTraits::Adapt(CreateRegisterNamespaceTransaction(model::NamespaceType::Root, namespaceName));
		auto namespaceId = pTransaction->NamespaceId;
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(5u, test::GetFieldCount(view));
		AssertSharedRegisterNamespaceData(pTransaction->NamespaceType, Namespace_Base_Id, namespaceId, namespaceName, view);
		EXPECT_EQ(pTransaction->Duration.unwrap(), test::GetUint64(view, "duration"));
	}

	PLUGIN_TEST(CanMapChildRegisterNamespaceTransactionWithName) {
		// Arrange:
		std::string namespaceName("jabo38");
		auto pTransaction = TTraits::Adapt(CreateRegisterNamespaceTransaction(model::NamespaceType::Child, namespaceName));
		auto namespaceId = pTransaction->NamespaceId;
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(4u, test::GetFieldCount(view));
		AssertSharedRegisterNamespaceData(pTransaction->NamespaceType, pTransaction->ParentId, namespaceId, namespaceName, view);
	}

	// endregion
}}}
