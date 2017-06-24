#include "src/AggregateMapper.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/test/mongo/MongoTransactionPluginTestUtils.h"
#include "tests/test/mongo/mocks/MockTransactionMapper.h"
#include "tests/TestHarness.h"

#define TEST_CLASS AggregateMapperTests

namespace catapult { namespace mongo { namespace plugins {
	using TransactionType = model::AggregateTransaction;
	using EmbeddedEntityType = mocks::EmbeddedMockTransaction;

	namespace {
		auto AllocateAggregateTransaction(uint16_t numTransactions, uint16_t numCosignatures) {
			uint32_t entitySize = sizeof(TransactionType)
					+ numTransactions * sizeof(EmbeddedEntityType)
					+ numCosignatures * sizeof(model::Cosignature);
			std::unique_ptr<TransactionType> pTransaction(reinterpret_cast<TransactionType*>(::operator new(entitySize)));
			pTransaction->Size = entitySize;
			pTransaction->PayloadSize = numTransactions * sizeof(EmbeddedEntityType);
			return pTransaction;
		}
	}

	// region basic

	TEST(TEST_CLASS, CanCreatePlugin) {
		// Act:
		MongoTransactionRegistry registry;
		auto pPlugin = CreateAggregateTransactionMongoPlugin(registry);

		// Assert:
		EXPECT_EQ(model::EntityType::Aggregate, pPlugin->type());
	}

	TEST(TEST_CLASS, PluginDoesNotSupportEmbedding) {
		// Arrange:
		MongoTransactionRegistry registry;
		auto pPlugin = CreateAggregateTransactionMongoPlugin(registry);

		// Act + Assert:
		EXPECT_FALSE(pPlugin->supportsEmbedding());
		EXPECT_THROW(pPlugin->embeddedPlugin(), catapult_runtime_error);
	}

	// endregion

	// region streamTransaction

	namespace {
		void AssertCanMapAggregateTransactionWithCosignatures(uint16_t numCosignatures) {
			// Arrange: create aggregate with a single sub transaction
			auto pTransaction = AllocateAggregateTransaction(1, numCosignatures);

			// - create and copy cosignatures
			auto cosignatures = test::GenerateRandomDataVector<model::Cosignature>(numCosignatures);
			std::memcpy(pTransaction->CosignaturesPtr(), cosignatures.data(), numCosignatures * sizeof(model::Cosignature));

			// - create the plugin
			MongoTransactionRegistry registry;
			auto pPlugin = CreateAggregateTransactionMongoPlugin(registry);

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert: only cosignatures should be present and they should always be present (even if there are no cosignatures)
			EXPECT_EQ(1u, test::GetFieldCount(view));

			auto dbCosignatures = view["cosignatures"].get_array().value;
			ASSERT_EQ(numCosignatures, std::distance(dbCosignatures.cbegin(), dbCosignatures.cend()));
			const auto* pCosignature = cosignatures.data();
			auto iter = dbCosignatures.cbegin();
			for (auto i = 0u; i < numCosignatures; ++i) {
				auto cosignatureView = iter->get_document().view();
				EXPECT_EQ(
						test::ToHexString(pCosignature->Signer),
						test::ToHexString(test::GetBinary(cosignatureView, "signer"), Key_Size));
				EXPECT_EQ(
						test::ToHexString(pCosignature->Signature),
						test::ToHexString(test::GetBinary(cosignatureView, "signature"), Signature_Size));
				++pCosignature;
				++iter;
			}
		}
	}

	TEST(TEST_CLASS, CanMapAggregateTransactionWithoutCosignatures) {
		// Assert:
		AssertCanMapAggregateTransactionWithCosignatures(0);
	}

	TEST(TEST_CLASS, CanMapAggregateTransactionWithSingleCosignature) {
		// Assert:
		AssertCanMapAggregateTransactionWithCosignatures(1);
	}

	TEST(TEST_CLASS, CanMapAggregateTransactionWithMultipleCosignatures) {
		// Assert:
		AssertCanMapAggregateTransactionWithCosignatures(3);
	}

	// endregion

	// region extractDependentDocuments

	namespace {
		void AssertExtractDependentDocuments(uint16_t numTransactions) {
			// Arrange: create aggregate with two cosignatures
			auto pTransaction = AllocateAggregateTransaction(numTransactions, 2);

			// - create and copy sub transactions
			auto subTransactions = test::GenerateRandomDataVector<EmbeddedEntityType>(numTransactions);
			for (auto& subTransaction : subTransactions) {
				subTransaction.Size = sizeof(EmbeddedEntityType);
				subTransaction.Type = EmbeddedEntityType::Entity_Type;
				subTransaction.Data.Size = 0;
			}

			std::memcpy(pTransaction->TransactionsPtr(), subTransactions.data(), numTransactions * sizeof(EmbeddedEntityType));

			// - create the plugin
			MongoTransactionRegistry registry;
			registry.registerPlugin(mocks::CreateMockTransactionMongoPlugin());
			auto pPlugin = CreateAggregateTransactionMongoPlugin(registry);

			// Act:
			auto entityHash = test::GenerateRandomData<Hash256_Size>();
			auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();
			auto metadata = MongoTransactionMetadata(entityHash, merkleComponentHash, Height(12), 2);
			auto documents = pPlugin->extractDependentDocuments(*pTransaction, metadata);

			// Assert:
			ASSERT_EQ(numTransactions, documents.size());

			for (auto i = 0u; i < numTransactions; ++i) {
				const auto& subTransaction = subTransactions[i];

				// - the document has meta and transaction parts
				auto view = documents[i].view();
				EXPECT_EQ(2u, test::GetFieldCount(view));

				auto metaView = view["meta"].get_document().view();
				EXPECT_EQ(3u, test::GetFieldCount(metaView));
				EXPECT_EQ(Height(12), Height(test::GetUint64(metaView, "height")));
				EXPECT_EQ(metadata.ObjectId, metaView["aggregateId"].get_oid().value);
				EXPECT_EQ(i, test::GetUint32(metaView, "index"));

				// - the mock mapper adds a recipient entry
				auto subTransactionView = view["transaction"].get_document().view();
				EXPECT_EQ(4u, test::GetFieldCount(subTransactionView));
				test::AssertEqualEmbeddedEntityData(subTransaction, subTransactionView);
				EXPECT_EQ(
						test::ToHexString(subTransaction.Recipient),
						test::ToHexString(test::GetBinary(subTransactionView, "recipient"), Key_Size));
			}
		}
	}

	TEST(TEST_CLASS, NoDependentDocumentsAreExtractedWhenThereAreNoSubTransactions) {
		// Assert:
		AssertExtractDependentDocuments(0);
	}

	TEST(TEST_CLASS, SingleDependentDocumentIsExtractedWhenThereIsOneSubTransaction) {
		// Assert:
		AssertExtractDependentDocuments(1);
	}

	TEST(TEST_CLASS, MultipleDependentDocumentsAreExtractedWhenThereAreMultipleSubTransactions) {
		// Assert:
		AssertExtractDependentDocuments(3);
	}

	// endregion
}}}
