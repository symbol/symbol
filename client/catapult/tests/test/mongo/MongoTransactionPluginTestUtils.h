#pragma once
#include "plugins/mongo/coremongo/src/MongoTransactionPlugin.h"
#include "catapult/model/Transaction.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"

namespace catapult { namespace test {

	/// Converts a transaction (\a pTransaction) to an embedded transaction.
	template<typename TEmbeddedTransaction, typename TTransaction>
	std::unique_ptr<TEmbeddedTransaction> ConvertToEmbedded(std::unique_ptr<TTransaction>&& pTransaction) {
		uint32_t size = pTransaction->Size - sizeof(model::Transaction) + sizeof(model::EmbeddedEntity);
		std::unique_ptr<TEmbeddedTransaction> pEmbeddedTransaction(reinterpret_cast<TEmbeddedTransaction*>(::operator new(size)));
		pEmbeddedTransaction->Size = size;

		const auto* pTransactionData = reinterpret_cast<const uint8_t*>(pTransaction.get());
		auto* pEmbeddedTransactionData = reinterpret_cast<uint8_t*>(pEmbeddedTransaction.get());

		// copy embedded entity body (excluding header Size and Signature)
		memcpy(
				pEmbeddedTransactionData + sizeof(uint32_t),
				pTransactionData + sizeof(uint32_t) + sizeof(Signature),
				sizeof(model::EmbeddedEntity) - sizeof(uint32_t));

		// copy custom transaction payload
		memcpy(
				pEmbeddedTransactionData + sizeof(model::EmbeddedEntity),
				pTransactionData + sizeof(model::Transaction),
				size - sizeof(model::EmbeddedEntity));
		return pEmbeddedTransaction;
	}

/// Defines traits for mongo transaction plugin based tests for \a NAME transaction without adaptation support.
#define DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(NAME) \
	struct RegularTraits { \
		using TransactionType = model::NAME##Transaction; \
		\
		static auto CreatePlugin() { \
			return Create##NAME##TransactionMongoPlugin(); \
		} \
	}; \
	\
	struct EmbeddedTraits { \
		using TransactionType = model::Embedded##NAME##Transaction; \
		\
		static auto CreatePlugin() { \
			return test::ExtractEmbeddedPlugin(RegularTraits::CreatePlugin()); \
		} \
	};

/// Defines traits for mongo transaction plugin based tests for \a NAME transaction.
#define DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(NAME) \
	struct RegularTraits { \
		using TransactionType = model::NAME##Transaction; \
		\
		static auto CreatePlugin() { \
			return Create##NAME##TransactionMongoPlugin(); \
		} \
		\
		static auto Adapt(std::unique_ptr<TransactionType>&& pTransaction) { \
			return std::move(pTransaction); \
		} \
	}; \
	\
	struct EmbeddedTraits { \
		using TransactionType = model::Embedded##NAME##Transaction; \
		\
		static auto CreatePlugin() { \
			return test::ExtractEmbeddedPlugin(RegularTraits::CreatePlugin()); \
		} \
		\
		static auto Adapt(std::unique_ptr<model::NAME##Transaction>&& pTransaction) { \
			/* TODO: this is temporary until builders expose buildEmbedded! */ \
			return test::ConvertToEmbedded<TransactionType>(std::move(pTransaction)); \
		} \
	};

	/// Asserts that a mongo transaction plugin does not produce any dependent documents.
	template<typename TTraits>
	void AssertDependentDocumentsAreNotSupported() {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		auto entityHash = test::GenerateRandomData<Hash256_Size>();
		auto merkleComponentHash = test::GenerateRandomData<Hash256_Size>();
		auto metadata = mongo::plugins::MongoTransactionMetadata(entityHash, merkleComponentHash);

		// Act:
		auto documents = pPlugin->extractDependentDocuments(transaction, metadata);

		// Assert:
		EXPECT_TRUE(documents.empty());
	}

/// Defines basic tests for a mongo transaction plugin with \a TYPE in \a TEST_CLASS.
#define DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TYPE) \
	DEFINE_COMMON_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TYPE) \
	\
	TEST(TEST_CLASS, DependentDocumentsAreNotSupported) { \
		test::AssertDependentDocumentsAreNotSupported<RegularTraits>(); \
	}
}}
