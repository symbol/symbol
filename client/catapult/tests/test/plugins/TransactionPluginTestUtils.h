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
		return std::shared_ptr<typename std::remove_reference<decltype(*pEmbeddedPlugin)>::type>(
				pEmbeddedPlugin,
				[pPlugin = std::shared_ptr<TTransactionPlugin>(std::move(pPlugin))](auto*) {});
	}

/// Defines traits for transaction plugin based tests for \a NAME transaction.
#define DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(NAME) \
	struct RegularTraits { \
		using TransactionType = model::NAME##Transaction; \
		\
		static auto CreatePlugin() { \
			return Create##NAME##TransactionPlugin(); \
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

/// Defines traits for transaction plugin based tests for \a NAME transaction requiring configuration of type \a CONFIG_TYPE.
#define TRANSACTION_PLUGIN_WITH_CONFIG_TEST_TRAITS(NAME, CONFIG_TYPE) \
	struct RegularTraits { \
		using TransactionType = model::NAME##Transaction; \
		\
		static auto CreatePlugin(const CONFIG_TYPE& config) { \
			return Create##NAME##TransactionPlugin(config); \
		} \
	}; \
	\
	struct EmbeddedTraits { \
		using TransactionType = model::Embedded##NAME##Transaction; \
		\
		static auto CreatePlugin(const CONFIG_TYPE& config) { \
			return test::ExtractEmbeddedPlugin(RegularTraits::CreatePlugin(config)); \
		} \
	};

/// Defines a test named \a TEST_NAME for both transaction and embedded transaction plugins.
#define PLUGIN_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TransactionPlugin_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, EmbeddedTransactionPlugin_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

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

/// Defines common tests for a transaction plugin with \a TYPE in \a TEST_CLASS.
/// \note \a TYPE is first __VA_ARGS__ parameter.
/// \note These tests should be supported by ALL embeddable transaction plugins.
#define DEFINE_COMMON_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, ...) \
	TEST(TEST_CLASS, CanCreateTransactionPluginWithEmbeddingSupport) { \
		test::AssertCanCreateTransactionPluginWithEmbeddingSupport<RegularTraits>(__VA_ARGS__); \
	} \
	\
	PLUGIN_TEST(CanCreatePlugin) { \
		test::AssertCanCreatePlugin<TTraits>(__VA_ARGS__); \
	}

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
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 12 - model::VerifiableEntity::Header_Size, buffer.Size);
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

/// Defines basic tests for a transaction plugin with \a TYPE in \a TEST_CLASS.
/// \note \a TYPE is first __VA_ARGS__ parameter.
#define DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, ...) \
	DEFINE_COMMON_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, __VA_ARGS__) \
	\
	TEST(TEST_CLASS, CanExtractPrimaryDataBuffer) { \
		test::AssertCanExtractPrimaryDataBuffer<RegularTraits>(__VA_ARGS__); \
	} \
	\
	TEST(TEST_CLASS, MerkleSupplementaryBuffersAreEmpty) { \
		test::AssertMerkleSupplementaryBuffersAreEmpty<RegularTraits>(__VA_ARGS__); \
	}
}}
