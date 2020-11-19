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
#include "catapult/model/TransactionPlugin.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region utils

	/// Extracts an embedded transaction plugin from a transaction plugin (\a pPlugin).
	template<typename TTransactionPlugin>
	auto ExtractEmbeddedPlugin(std::unique_ptr<TTransactionPlugin>&& pPlugin) {
		if (!pPlugin->supportsEmbedding())
			CATAPULT_THROW_RUNTIME_ERROR("plugin must support embedding");

		const auto* pEmbeddedPlugin = &pPlugin->embeddedPlugin();
		return std::shared_ptr<std::remove_reference_t<decltype(*pEmbeddedPlugin)>>(
				pEmbeddedPlugin,
				[pPlugin = std::shared_ptr<TTransactionPlugin>(std::move(pPlugin))](auto*) {});
	}

	// endregion

	// region PLUGIN_TEST

/// Defines a test named \a TEST_NAME for both transaction and embedded transaction plugins using traits prefixed by \a TRAITS_PREFIX
/// and test name postfixed by \a TEST_POSTFIX.
#define PLUGIN_TEST_WITH_PREFIXED_TRAITS(TEST_NAME, TRAITS_PREFIX, TEST_POSTFIX) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME##TEST_POSTFIX)(); \
	TEST(TEST_CLASS, TransactionPlugin_##TEST_NAME##TEST_POSTFIX) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME##TEST_POSTFIX)<TRAITS_PREFIX##RegularTraits>(); \
	} \
	TEST(TEST_CLASS, EmbeddedTransactionPlugin_##TEST_NAME##TEST_POSTFIX) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME##TEST_POSTFIX)<TRAITS_PREFIX##EmbeddedTraits>(); \
	} \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME##TEST_POSTFIX)()

/// Defines a test named \a TEST_NAME for both transaction and embedded transaction plugins.
#define PLUGIN_TEST(TEST_NAME) PLUGIN_TEST_WITH_PREFIXED_TRAITS(TEST_NAME, ,)

	// endregion

	// region transaction plugin and mongo transaction plugin shared tests

	/// Shared TransactionPlugin and MongoTransactionPlugin tests.
	template<typename TTraits>
	class SharedTransactionPluginTests {
	public:
		/// Asserts that a transaction plugin can be created and has the expected \a type.
		template<typename... TArgs>
		static void AssertCanCreatePlugin(model::EntityType type, TArgs&& ...args) {
			// Act:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			// Assert:
			EXPECT_EQ(type, pPlugin->type());
		}

		/// Asserts that a transaction plugin supports embeddings of \a type.
		template<typename... TArgs>
		static void AssertPluginSupportsEmbedding(model::EntityType type, TArgs&& ...args) {
			// Act:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			// Assert:
			ASSERT_TRUE(pPlugin->supportsEmbedding());
			EXPECT_EQ(type, pPlugin->embeddedPlugin().type());
		}
	};

/// Defines common tests for a transaction plugin with \a TYPE in \a TEST_CLASS using traits prefixed by \a TRAITS_PREFIX
/// and test name postfixed by \a TEST_POSTFIX.
/// \note \a TYPE is first __VA_ARGS__ parameter.
/// \note These tests should be supported by ALL (regular and mongo) embeddable transaction plugins.
///
/// Coverage:
/// - regular and embedded: { type }
/// - regular: { supportsEmbedding, embeddedPlugin }
#define DEFINE_SHARED_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, ...) \
	PLUGIN_TEST_WITH_PREFIXED_TRAITS(CanCreatePlugin, TRAITS_PREFIX, TEST_POSTFIX) { \
		test::SharedTransactionPluginTests<TTraits>::AssertCanCreatePlugin(__VA_ARGS__); \
	} \
	TEST(TEST_CLASS, PluginSupportsEmbedding##TEST_POSTFIX) { \
		test::SharedTransactionPluginTests<TRAITS_PREFIX##RegularTraits>::AssertPluginSupportsEmbedding(__VA_ARGS__); \
	}

	// endregion
}}
