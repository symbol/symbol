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

	/// Publishes \a transaction notifications to \a sub using \a plugin.
	template<typename TTransactionPlugin, typename TTransaction>
	void PublishTransaction(const TTransactionPlugin& plugin, const TTransaction& transaction, model::NotificationSubscriber& sub) {
		plugin.publish(transaction, sub);
	}

	// endregion

	// region transaction plugin test traits

/// Defines traits for transaction plugin based tests for \a NAME transaction using traits prefixed by \a TRAITS_PREFIX
/// with support for transaction versions \a MIN_VERSION to \a MAX_VERSION.
#define DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(NAME, MIN_VERSION, MAX_VERSION, TRAITS_PREFIX) \
	struct TRAITS_PREFIX##RegularTraits { \
		using TransactionType = model::NAME##Transaction; \
		static constexpr auto Min_Supported_Version = MIN_VERSION; \
		static constexpr auto Max_Supported_Version = MAX_VERSION; \
		\
		static auto CreatePlugin() { \
			return Create##NAME##TransactionPlugin(); \
		} \
	}; \
	\
	struct TRAITS_PREFIX##EmbeddedTraits { \
		using TransactionType = model::Embedded##NAME##Transaction; \
		static constexpr auto Min_Supported_Version = MIN_VERSION; \
		static constexpr auto Max_Supported_Version = MAX_VERSION; \
		\
		static auto CreatePlugin() { \
			return test::ExtractEmbeddedPlugin(TRAITS_PREFIX##RegularTraits::CreatePlugin()); \
		} \
	};

/// Defines traits for transaction plugin based tests for \a NAME transaction requiring configuration of type \a CONFIG_TYPE
/// using traits prefixed by \a TRAITS_PREFIX with support for transaction versions \a MIN_VERSION to \a MAX_VERSION.
#define DEFINE_TRANSACTION_PLUGIN_WITH_CONFIG_TEST_TRAITS(NAME, CONFIG_TYPE, MIN_VERSION, MAX_VERSION, TRAITS_PREFIX) \
	struct TRAITS_PREFIX##RegularTraits { \
		using TransactionType = model::NAME##Transaction; \
		static constexpr auto Min_Supported_Version = MIN_VERSION; \
		static constexpr auto Max_Supported_Version = MAX_VERSION; \
		\
		static auto CreatePlugin(const CONFIG_TYPE& config) { \
			return Create##NAME##TransactionPlugin(config); \
		} \
	}; \
	\
	struct TRAITS_PREFIX##EmbeddedTraits { \
		using TransactionType = model::Embedded##NAME##Transaction; \
		static constexpr auto Min_Supported_Version = MIN_VERSION; \
		static constexpr auto Max_Supported_Version = MAX_VERSION; \
		\
		static auto CreatePlugin(const CONFIG_TYPE& config) { \
			return test::ExtractEmbeddedPlugin(TRAITS_PREFIX##RegularTraits::CreatePlugin(config)); \
		} \
	};

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

	// region transaction and mongo shared tests

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
#define DEFINE_SHARED_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, ...) \
	PLUGIN_TEST_WITH_PREFIXED_TRAITS(CanCreatePlugin, TRAITS_PREFIX, TEST_POSTFIX) { \
		test::SharedTransactionPluginTests<TTraits>::AssertCanCreatePlugin(__VA_ARGS__); \
	} \
	TEST(TEST_CLASS, PluginSupportsEmbedding##TEST_POSTFIX) { \
		test::SharedTransactionPluginTests<TRAITS_PREFIX##RegularTraits>::AssertPluginSupportsEmbedding(__VA_ARGS__); \
	}

	// endregion

	// region transaction tests

	/// TransactionPlugin tests.
	template<typename TTraits>
	class TransactionPluginTests {
	public:
		/// Asserts that a transaction plugin returns expected attributes.
		template<typename... TArgs>
		static void AssertAttributesReturnsCorrectValues(model::EntityType, TArgs&& ...args) {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			// Act:
			auto attributes = pPlugin->attributes();

			// Assert:
			EXPECT_EQ(TTraits::Min_Supported_Version, attributes.MinVersion);
			EXPECT_EQ(TTraits::Max_Supported_Version, attributes.MaxVersion);

			// - zero denotes default lifetime should be used
			EXPECT_EQ(utils::TimeSpan(), attributes.MaxLifetime);
		}

		/// Asserts that a primary data buffer can be extracted from a transaction plugin.
		template<typename... TArgs>
		static void AssertCanExtractPrimaryDataBuffer(model::EntityType, TArgs&& ...args) {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			typename TTraits::TransactionType transaction;
			transaction.Size = sizeof(typename TTraits::TransactionType) + 12;

			// Act:
			auto buffer = pPlugin->dataBuffer(transaction);

			// Assert:
			EXPECT_EQ(test::AsVoidPointer(&transaction.Version), test::AsVoidPointer(buffer.pData));
			ASSERT_EQ(sizeof(typename TTraits::TransactionType) + 12 - model::VerifiableEntity::Header_Size, buffer.Size);
		}

		/// Asserts that merkle supplementary buffers are empty for a transaction plugin.
		template<typename... TArgs>
		static void AssertMerkleSupplementaryBuffersAreEmpty(model::EntityType, TArgs&& ...args) {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			typename TTraits::TransactionType transaction;
			test::FillWithRandomData(transaction.Signer);

			// Act:
			auto buffers = pPlugin->merkleSupplementaryBuffers(transaction);

			// Assert:
			EXPECT_TRUE(buffers.empty());
		}

		/// Asserts that top-level block embedding is supported.
		template<typename... TArgs>
		static void AssertPluginSupportsTopLevel(model::EntityType, TArgs&& ...args) {
			// Act:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			// Assert:
			EXPECT_TRUE(pPlugin->supportsTopLevel());
		}

		/// Asserts that top-level block embedding is not supported.
		template<typename... TArgs>
		static void AssertPluginDoesNotSupportTopLevel(model::EntityType, TArgs&& ...args) {
			// Act:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			// Assert:
			EXPECT_FALSE(pPlugin->supportsTopLevel());
		}
	};

/// Defines basic tests for a transaction plugin with \a TYPE in \a TEST_CLASS using traits prefixed by \a TRAITS_PREFIX
/// and test name postfixed by \a TEST_POSTFIX.
/// \note \a TYPE is first __VA_ARGS__ parameter.
#define DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ALL(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, ...) \
	DEFINE_SHARED_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, __VA_ARGS__) \
	\
	PLUGIN_TEST_WITH_PREFIXED_TRAITS(AttributesReturnsCorrectValues, TRAITS_PREFIX, TEST_POSTFIX) { \
		test::TransactionPluginTests<TTraits>::AssertAttributesReturnsCorrectValues(__VA_ARGS__); \
	} \
	TEST(TEST_CLASS, CanExtractPrimaryDataBuffer##TEST_POSTFIX) { \
		test::TransactionPluginTests<TRAITS_PREFIX##RegularTraits>::AssertCanExtractPrimaryDataBuffer(__VA_ARGS__); \
	} \
	TEST(TEST_CLASS, MerkleSupplementaryBuffersAreEmpty##TEST_POSTFIX) { \
		test::TransactionPluginTests<TRAITS_PREFIX##RegularTraits>::AssertMerkleSupplementaryBuffersAreEmpty(__VA_ARGS__); \
	}

/// Defines basic tests for a transaction plugin with \a TYPE in \a TEST_CLASS using traits prefixed by \a TRAITS_PREFIX
/// and test name postfixed by \a TEST_POSTFIX.
/// \note \a TYPE is first __VA_ARGS__ parameter.
/// \note This is intended for TransactionPluginOptions::Default.
#define DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, ...) \
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ALL(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, __VA_ARGS__) \
	\
	TEST(TEST_CLASS, PluginSupportsTopLevel##TEST_POSTFIX) { \
		test::TransactionPluginTests<TRAITS_PREFIX##RegularTraits>::AssertPluginSupportsTopLevel(__VA_ARGS__); \
	}

/// Defines basic tests for a transaction plugin with \a TYPE in \a TEST_CLASS using traits prefixed by \a TRAITS_PREFIX
/// and test name postfixed by \a TEST_POSTFIX.
/// \note \a TYPE is first __VA_ARGS__ parameter.
/// \note This is intended for TransactionPluginOptions::Only_Embeddable.
#define DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ONLY_EMBEDDABLE(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, ...) \
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ALL(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, __VA_ARGS__) \
	\
	TEST(TEST_CLASS, PluginDoesNotSupportTopLevel##TEST_POSTFIX) { \
		test::TransactionPluginTests<TRAITS_PREFIX##RegularTraits>::AssertPluginDoesNotSupportTopLevel(__VA_ARGS__); \
	}

	// endregion
}}
