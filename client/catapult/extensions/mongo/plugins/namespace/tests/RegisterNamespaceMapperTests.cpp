#include "src/RegisterNamespaceMapper.h"
#include "sdk/src/builders/RegisterNamespaceBuilder.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS RegisterNamespaceMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(RegisterNamespace);

		auto CreateRegisterNamespaceTransactionBuilder(
				const Key& signer,
				model::NamespaceType namespaceType,
				const std::string& namespaceName) {
			builders::RegisterNamespaceBuilder builder(model::NetworkIdentifier::Mijin_Test, signer, namespaceName);

			if (model::NamespaceType::Root == namespaceType)
				builder.setDuration(test::GenerateRandomValue<BlockDuration>());
			else
				builder.setParentId(test::GenerateRandomValue<NamespaceId>());

			return builder;
		}

		void AssertSharedRegisterNamespaceData(
				model::NamespaceType namespaceType,
				NamespaceId id,
				const std::string& namespaceName,
				const bsoncxx::document::view& dbTransaction) {
			// Assert:
			EXPECT_EQ(namespaceType, static_cast<model::NamespaceType>(test::GetUint32(dbTransaction, "namespaceType")));
			EXPECT_EQ(id.unwrap(), test::GetUint64(dbTransaction, "namespaceId"));

			auto dbName = dbTransaction["name"].get_binary();
			EXPECT_EQ(namespaceName.size(), dbName.size);
			EXPECT_EQ(
					test::ToHexString(reinterpret_cast<const uint8_t*>(namespaceName.data()), namespaceName.size()),
					test::ToHexString(dbName.bytes, dbName.size));
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, model::Entity_Type_Register_Namespace)

	// region streamTransaction

	PLUGIN_TEST(CannotMapRegisterNamespaceTransactionWithoutName) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		transaction.Size = sizeof(typename TTraits::TransactionType);
		transaction.Type = model::Entity_Type_Register_Namespace;
		transaction.NamespaceType = model::NamespaceType::Root;
		transaction.NamespaceNameSize = 0;

		auto pPlugin = TTraits::CreatePlugin();

		// Act + Assert:
		mappers::bson_stream::document builder;
		EXPECT_THROW(pPlugin->streamTransaction(builder, transaction), catapult_runtime_error);
	}

	PLUGIN_TEST(CanMapRootRegisterNamespaceTransactionWithName) {
		// Arrange:
		std::string namespaceName("jabo38");
		auto signer = test::GenerateRandomData<Key_Size>();
		auto pTransaction = TTraits::Adapt(CreateRegisterNamespaceTransactionBuilder(signer, model::NamespaceType::Root, namespaceName));
		auto namespaceId = pTransaction->NamespaceId;
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(4u, test::GetFieldCount(view));
		AssertSharedRegisterNamespaceData(pTransaction->NamespaceType, namespaceId, namespaceName, view);
		EXPECT_EQ(pTransaction->Duration.unwrap(), test::GetUint64(view, "duration"));
	}

	PLUGIN_TEST(CanMapChildRegisterNamespaceTransactionWithName) {
		// Arrange:
		std::string namespaceName("jabo38");
		auto signer = test::GenerateRandomData<Key_Size>();
		auto pTransaction = TTraits::Adapt(CreateRegisterNamespaceTransactionBuilder(signer, model::NamespaceType::Child, namespaceName));
		auto namespaceId = pTransaction->NamespaceId;
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(4u, test::GetFieldCount(view));
		AssertSharedRegisterNamespaceData(pTransaction->NamespaceType, namespaceId, namespaceName, view);
		EXPECT_EQ(pTransaction->ParentId.unwrap(), test::GetUint64(view, "parentId"));
	}

	// endregion
}}}
