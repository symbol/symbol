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

/// Defines traits for transaction plugin based tests for \a NAME transaction using traits prefixed by \a TRAITS_PREFIX
/// with support for transaction versions \a MIN_VERSION to \a MAX_VERSION.
#define DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS_WITH_PREFIXED_TRAITS(NAME, MIN_VERSION, MAX_VERSION, TRAITS_PREFIX) \
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

/// Defines traits for transaction plugin based tests for \a NAME transaction
/// with support for transaction versions \a MIN_VERSION to \a MAX_VERSION.
#define DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(NAME, MIN_VERSION, MAX_VERSION) \
	DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS_WITH_PREFIXED_TRAITS(NAME, MIN_VERSION, MAX_VERSION,)

/// Defines traits for transaction plugin based tests for \a NAME transaction requiring configuration of type \a CONFIG_TYPE
/// with support for transaction versions \a MIN_VERSION to \a MAX_VERSION.
#define TRANSACTION_PLUGIN_WITH_CONFIG_TEST_TRAITS(NAME, CONFIG_TYPE, MIN_VERSION, MAX_VERSION) \
	struct RegularTraits { \
		using TransactionType = model::NAME##Transaction; \
		static constexpr auto Min_Supported_Version = MIN_VERSION; \
		static constexpr auto Max_Supported_Version = MAX_VERSION; \
		\
		static auto CreatePlugin(const CONFIG_TYPE& config) { \
			return Create##NAME##TransactionPlugin(config); \
		} \
	}; \
	\
	struct EmbeddedTraits { \
		using TransactionType = model::Embedded##NAME##Transaction; \
		static constexpr auto Min_Supported_Version = MIN_VERSION; \
		static constexpr auto Max_Supported_Version = MAX_VERSION; \
		\
		static auto CreatePlugin(const CONFIG_TYPE& config) { \
			return test::ExtractEmbeddedPlugin(RegularTraits::CreatePlugin(config)); \
		} \
	};

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

	/// Asserts that a transaction plugin supports embeddings of \a type.
	template<typename TTraits, typename... TArgs>
	void AssertCanCreateTransactionPluginWithEmbeddingSupport(model::EntityType type, TArgs&& ...args) {
		// Act:
		auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

		// Assert:
		ASSERT_TRUE(pPlugin->supportsEmbedding());
		EXPECT_EQ(type, pPlugin->embeddedPlugin().type());
	}

	/// Asserts that a transaction plugin can be created and has the expected \a type.
	template<typename TTraits, typename... TArgs>
	void AssertCanCreatePlugin(model::EntityType type, TArgs&& ...args) {
		// Act:
		auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

		// Assert:
		EXPECT_EQ(type, pPlugin->type());
	}

	/// Asserts that a transaction plugin supports expected versions.
	template<typename TTraits, typename... TArgs>
	void AssertSupportedVersionsReturnsCorrectVersions(model::EntityType, TArgs&& ...args) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

		// Act:
		auto supportedVersions = pPlugin->supportedVersions();

		// Assert:
		auto minVersion = TTraits::Min_Supported_Version;
		auto maxVersion = TTraits::Max_Supported_Version;
		EXPECT_EQ(minVersion, supportedVersions.MinVersion);
		EXPECT_EQ(maxVersion, supportedVersions.MaxVersion);
	}

/// Defines common tests for a transaction plugin with \a TYPE in \a TEST_CLASS using traits prefixed by \a TRAITS_PREFIX
/// and test name postfixed by \a TEST_POSTFIX.
/// \note \a TYPE is first __VA_ARGS__ parameter.
/// \note These tests should be supported by ALL embeddable transaction plugins.
#define DEFINE_COMMON_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, ...) \
	TEST(TEST_CLASS, CanCreateTransactionPluginWithEmbeddingSupport##TEST_POSTFIX) { \
		test::AssertCanCreateTransactionPluginWithEmbeddingSupport<TRAITS_PREFIX##RegularTraits>(__VA_ARGS__); \
	} \
	\
	PLUGIN_TEST_WITH_PREFIXED_TRAITS(CanCreatePlugin, TRAITS_PREFIX, TEST_POSTFIX) { \
		test::AssertCanCreatePlugin<TTraits>(__VA_ARGS__); \
	}

/// Defines common tests for a transaction plugin with \a TYPE in \a TEST_CLASS.
/// \note \a TYPE is first __VA_ARGS__ parameter.
/// \note These tests should be supported by ALL embeddable transaction plugins.
#define DEFINE_COMMON_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, ...) \
	DEFINE_COMMON_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(TEST_CLASS, , , __VA_ARGS__)

	/// Asserts that a primary data buffer can be extracted from a transaction plugin.
	template<typename TTraits, typename... TArgs>
	void AssertCanExtractPrimaryDataBuffer(model::EntityType, TArgs&& ...args) {
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
	template<typename TTraits, typename... TArgs>
	void AssertMerkleSupplementaryBuffersAreEmpty(model::EntityType, TArgs&& ...args) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction.Signer);

		// Act:
		auto buffers = pPlugin->merkleSupplementaryBuffers(transaction);

		// Assert:
		EXPECT_TRUE(buffers.empty());
	}

/// Defines basic tests for a transaction plugin with \a TYPE in \a TEST_CLASS using traits prefixed by \a TRAITS_PREFIX
/// and test name postfixed by \a TEST_POSTFIX.
/// \note \a TYPE is first __VA_ARGS__ parameter.
#define DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, ...) \
	DEFINE_COMMON_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, __VA_ARGS__) \
	\
	TEST(TEST_CLASS, CanExtractPrimaryDataBuffer##TEST_POSTFIX) { \
		test::AssertCanExtractPrimaryDataBuffer<TRAITS_PREFIX##RegularTraits>(__VA_ARGS__); \
	} \
	\
	TEST(TEST_CLASS, MerkleSupplementaryBuffersAreEmpty##TEST_POSTFIX) { \
		test::AssertMerkleSupplementaryBuffersAreEmpty<TRAITS_PREFIX##RegularTraits>(__VA_ARGS__); \
	} \
	\
	PLUGIN_TEST_WITH_PREFIXED_TRAITS(SupportedVersionsReturnsCorrectVersion, TRAITS_PREFIX, TEST_POSTFIX) { \
		test::AssertSupportedVersionsReturnsCorrectVersions<TTraits>(__VA_ARGS__); \
	}

/// Defines basic tests for a transaction plugin with \a TYPE in \a TEST_CLASS.
/// \note \a TYPE is first __VA_ARGS__ parameter.
#define DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, ...) \
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_WITH_PREFIXED_TRAITS(TEST_CLASS, , , __VA_ARGS__)
}}
