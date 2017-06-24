#include "src/MongoTransactionPluginFactory.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS MongoTransactionPluginFactoryTests

	namespace {
		constexpr auto Mock_Transaction_Type = static_cast<model::EntityType>(0x4FFF);

		template<typename TTransaction>
		void Stream(bsoncxx::builder::stream::document& builder, const TTransaction& transaction) {
			builder << "signer0" << transaction.Signer[0];
		}

		struct RegularTraits {
			using TransactionType = mocks::MockTransaction;

			static auto CreatePlugin() {
				return MongoTransactionPluginFactory::Create<mocks::MockTransaction, mocks::EmbeddedMockTransaction>(
						[](auto& builder, const auto& transaction) { Stream(builder, transaction); },
						[](auto& builder, const auto& transaction) { Stream(builder, transaction); });
			}
		};

		struct EmbeddedTraits {
			using TransactionType = mocks::EmbeddedMockTransaction;

			static auto CreatePlugin() {
				return MongoTransactionPluginFactory::CreateEmbedded<mocks::EmbeddedMockTransaction>(
						[](auto& builder, const auto& transaction) { Stream(builder, transaction); });
			}
		};

#define PLUGIN_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
	}

	TEST(TEST_CLASS, CanCreateTransactionPluginWithEmbeddingSupport) {
		// Act:
		auto pPlugin = RegularTraits::CreatePlugin();

		// Assert:
		ASSERT_TRUE(pPlugin->supportsEmbedding());
		EXPECT_EQ(Mock_Transaction_Type, pPlugin->embeddedPlugin().type());
	}

	TEST(TEST_CLASS, DependentDocumentsAreNotSupportedByDefault) {
		// Arrange:
		auto pPlugin = RegularTraits::CreatePlugin();

		RegularTraits::TransactionType transaction;
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();
		auto metadata = MongoTransactionMetadata(entityHash, merkleComponentHash);

		// Act:
		auto documents = pPlugin->extractDependentDocuments(transaction, metadata);

		// Assert:
		EXPECT_TRUE(documents.empty());
	}

	PLUGIN_TEST(CanCreatePlugin) {
		// Act:
		auto pPlugin = TTraits::CreatePlugin();

		// Assert:
		EXPECT_EQ(Mock_Transaction_Type, pPlugin->type());
	}

	PLUGIN_TEST(CanStreamTransaction) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();
		bsoncxx::builder::stream::document builder;

		typename TTraits::TransactionType transaction;
		transaction.Signer[0] = 0x57;

		// Act:
		pPlugin->streamTransaction(builder, transaction);
		auto dbTransaction = builder << bsoncxx::builder::stream::finalize;

		// Assert:
		auto view = dbTransaction.view();
		EXPECT_EQ(1u, test::GetFieldCount(view));
		EXPECT_EQ(0x57, test::GetUint32(view, "signer0"));
	}
}}}
