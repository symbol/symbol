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

#pragma once
#include "mongo/src/MongoTransactionPlugin.h"
#include "catapult/model/Transaction.h"
#include "tests/test/plugins/SharedTransactionPluginTests.h"

namespace catapult { namespace test {

/// Defines traits for mongo transaction plugin based tests for \a NAME transaction without adaptation support
/// using traits prefixed by \a TRAITS_PREFIX.
#define DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(NAME, TRAITS_PREFIX) \
	struct TRAITS_PREFIX##RegularTraits { \
		using TransactionType = model::NAME##Transaction; \
		\
		static auto CreatePlugin() { \
			return Create##NAME##TransactionMongoPlugin(); \
		} \
	}; \
	\
	struct TRAITS_PREFIX##EmbeddedTraits { \
		using TransactionType = model::Embedded##NAME##Transaction; \
		\
		static auto CreatePlugin() { \
			return test::ExtractEmbeddedPlugin(TRAITS_PREFIX##RegularTraits::CreatePlugin()); \
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

	/// MongoTransactionPlugin tests.
	template<typename TTraits>
	class MongoTransactionPluginTests {
	public:
		/// Asserts that a mongo transaction plugin does not produce any dependent documents.
		static void AssertDependentDocumentsAreNotSupported() {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin();

			typename TTraits::TransactionType transaction;
			model::TransactionElement transactionElement(transaction);
			transactionElement.OptionalExtractedAddresses = std::make_shared<model::UnresolvedAddressSet>();
			auto metadata = mongo::MongoTransactionMetadata(transactionElement);

			// Act:
			auto documents = pPlugin->extractDependentDocuments(transaction, metadata);

			// Assert:
			EXPECT_TRUE(documents.empty());
		}
	};

/// Defines basic tests for a mongo transaction plugin with \a TYPE in \a TEST_CLASS using traits prefixed by \a TRAITS_PREFIX
/// and test name postfixed by \a TEST_POSTFIX.
///
/// Coverage:
/// - regular and embedded: { type }
/// - regular: { supportsEmbedding, embeddedPlugin, extractDependentDocuments }
/// - uncovered (regular and embedded): { streamTransaction }
#define DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, TYPE) \
	DEFINE_SHARED_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, TYPE) \
	\
	TEST(TEST_CLASS, DependentDocumentsAreNotSupported##TEST_POSTFIX) { \
		test::MongoTransactionPluginTests<TRAITS_PREFIX##RegularTraits>::AssertDependentDocumentsAreNotSupported(); \
	}
}}
