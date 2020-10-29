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
#include "SharedTransactionPluginTests.h"

namespace catapult { namespace test {

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

	// region transaction plugin tests

	/// TransactionPlugin tests.
	template<typename TTraits>
	class TransactionPluginTests {
	private:
		template<typename TPlugin>
		static bool IsSizeValidDispatcher(const TPlugin& plugin, typename TTraits::TransactionType& transaction, uint32_t size) {
			transaction.Size = size;
			return plugin.isSizeValid(transaction);
		}

	public:
		/// Asserts that is size valid calculation delegates to static transaction function.
		template<typename... TArgs>
		static void AssertIsSizeValidReturnsCorrectValuesWhenTransactionIsComplete(model::EntityType, TArgs&& ...args) {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			typename TTraits::TransactionType transaction;
			test::FillWithRandomData(transaction);
			auto expectedRealSize = static_cast<uint32_t>(TTraits::TransactionType::CalculateRealSize(transaction));

			// Act + Assert:
			EXPECT_FALSE(IsSizeValidDispatcher(*pPlugin, transaction, expectedRealSize - 1));
			EXPECT_TRUE(IsSizeValidDispatcher(*pPlugin, transaction, expectedRealSize));
			EXPECT_FALSE(IsSizeValidDispatcher(*pPlugin, transaction, expectedRealSize + 1));
		}

		/// Asserts that is size valid calculation delegates to static transaction function.
		template<typename... TArgs>
		static void AssertIsSizeValidReturnsCorrectValuesWhenTransactionIsIncomplete(model::EntityType, TArgs&& ...args) {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			model::SizePrefixedEntity entity;
			test::FillWithRandomData(entity);

			// Act + Assert:
			EXPECT_FALSE(IsSizeValidDispatcher(
					*pPlugin,
					static_cast<typename TTraits::TransactionType&>(entity),
					sizeof(model::SizePrefixedEntity)));
		}

		/// Asserts that transaction plugin returns correct attributes.
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

		/// Asserts that transaction plugin returns correct number of embedded transactions.
		template<typename... TArgs>
		static void AssertCanCountEmbeddedTransactions(model::EntityType, TArgs&& ...args) {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			typename TTraits::TransactionType transaction;
			test::FillWithRandomData(transaction);

			// Act:
			auto count = pPlugin->embeddedCount(transaction);

			// Assert:
			EXPECT_EQ(0u, count);
		}

		/// Asserts that a primary data buffer can be extracted from a transaction plugin.
		template<typename... TArgs>
		static void AssertCanExtractPrimaryDataBuffer(model::EntityType, TArgs&& ...args) {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			typename TTraits::TransactionType transaction;
			test::FillWithRandomData(transaction);
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
			test::FillWithRandomData(transaction);

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

		/// Asserts that additional required cosignatories are empty for a transaction plugin.
		template<typename... TArgs>
		static void AssertAdditionalRequiredCosignatoriesAreEmpty(model::EntityType, TArgs&& ...args) {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			typename TTraits::TransactionType transaction;
			test::FillWithRandomData(transaction);

			// Act:
			auto additionalCosignatories = pPlugin->additionalRequiredCosignatories(transaction);

			// Assert:
			EXPECT_TRUE(additionalCosignatories.empty());
		}
	};

/// Defines basic tests for a transaction plugin with \a TYPE in \a TEST_CLASS using traits prefixed by \a TRAITS_PREFIX
/// and test name postfixed by \a TEST_POSTFIX.
/// \note \a TYPE is first __VA_ARGS__ parameter.
#define DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ALL(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, ...) \
	DEFINE_SHARED_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, __VA_ARGS__) \
	\
	PLUGIN_TEST_WITH_PREFIXED_TRAITS(IsSizeValidReturnsCorrectValuesWhenTransactionIsComplete, TRAITS_PREFIX, TEST_POSTFIX) { \
		test::TransactionPluginTests<TTraits>::AssertIsSizeValidReturnsCorrectValuesWhenTransactionIsComplete(__VA_ARGS__); \
	} \
	PLUGIN_TEST_WITH_PREFIXED_TRAITS(IsSizeValidReturnsCorrectValuesWhenTransactionIsIncomplete, TRAITS_PREFIX, TEST_POSTFIX) { \
		test::TransactionPluginTests<TTraits>::AssertIsSizeValidReturnsCorrectValuesWhenTransactionIsIncomplete(__VA_ARGS__); \
	} \
	PLUGIN_TEST_WITH_PREFIXED_TRAITS(AttributesReturnsCorrectValues, TRAITS_PREFIX, TEST_POSTFIX) { \
		test::TransactionPluginTests<TTraits>::AssertAttributesReturnsCorrectValues(__VA_ARGS__); \
	} \
	TEST(TEST_CLASS, CanCountEmbeddedTransactions##TEST_POSTFIX) { \
		test::TransactionPluginTests<TRAITS_PREFIX##RegularTraits>::AssertCanCountEmbeddedTransactions(__VA_ARGS__); \
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
///
/// Coverage:
/// - regular and embedded: { type, isSizeValid, attributes }
/// - regular: { embeddedCount, dataBuffer, merkleSupplementaryBuffers, supportsTopLevel, supportsEmbedding, embeddedPlugin }
/// - embedded: { additionalRequiredCosignatories }
/// - uncovered (regular and embedded): { publish }
#define DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, ...) \
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ALL(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, __VA_ARGS__) \
	\
	TEST(TEST_CLASS, PluginSupportsTopLevel##TEST_POSTFIX) { \
		test::TransactionPluginTests<TRAITS_PREFIX##RegularTraits>::AssertPluginSupportsTopLevel(__VA_ARGS__); \
	} \
	TEST(TEST_CLASS, AdditionalRequiredCosignatoriesAreEmpty##TEST_POSTFIX) { \
		test::TransactionPluginTests<TRAITS_PREFIX##EmbeddedTraits>::AssertAdditionalRequiredCosignatoriesAreEmpty(__VA_ARGS__); \
	}

/// Defines basic tests for a transaction plugin with \a TYPE in \a TEST_CLASS using traits prefixed by \a TRAITS_PREFIX
/// and test name postfixed by \a TEST_POSTFIX.
/// \note \a TYPE is first __VA_ARGS__ parameter.
/// \note This is intended for TransactionPluginOptions::Only_Embeddable.
///
/// Coverage:
/// - regular and embedded: { type, isSizeValid, attributes }
/// - regular: { embeddedCount, dataBuffer, merkleSupplementaryBuffers, supportsTopLevel, supportsEmbedding, embeddedPlugin }
/// - uncovered (regular and embedded): { publish }
/// - uncovered (embedded): { additionalRequiredCosignatories }
#define DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ONLY_EMBEDDABLE(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, ...) \
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ALL(TEST_CLASS, TRAITS_PREFIX, TEST_POSTFIX, __VA_ARGS__) \
	\
	TEST(TEST_CLASS, PluginDoesNotSupportTopLevel##TEST_POSTFIX) { \
		test::TransactionPluginTests<TRAITS_PREFIX##RegularTraits>::AssertPluginDoesNotSupportTopLevel(__VA_ARGS__); \
	}

	// endregion
}}
