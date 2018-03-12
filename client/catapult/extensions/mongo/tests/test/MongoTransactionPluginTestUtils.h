#pragma once
#include "mongo/src/MongoTransactionPlugin.h"
#include "catapult/model/Transaction.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"

namespace catapult { namespace test {

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
		template<typename TBuilder> \
		static auto Adapt(const TBuilder& builder) { \
			return builder.build(); \
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
		template<typename TBuilder> \
		static auto Adapt(const TBuilder& builder) { \
			return builder.buildEmbedded(); \
		} \
	};

	/// Asserts that a mongo transaction plugin does not produce any dependent documents.
	template<typename TTraits>
	void AssertDependentDocumentsAreNotSupported() {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		model::TransactionElement transactionElement(transaction);
		auto metadata = mongo::MongoTransactionMetadata(transactionElement);

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
