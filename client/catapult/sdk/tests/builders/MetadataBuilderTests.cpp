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

#include "src/builders/AccountMetadataBuilder.h"
#include "src/builders/MosaicMetadataBuilder.h"
#include "src/builders/NamespaceMetadataBuilder.h"
#include "tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS MetadataBuilderTests

	namespace {
		struct TransactionProperties {
		public:
			TransactionProperties()
					: TargetAddress()
					, ScopedMetadataKey()
					, RawTargetId()
					, ValueSizeDelta()
			{}

		public:
			UnresolvedAddress TargetAddress;
			uint64_t ScopedMetadataKey;
			uint64_t RawTargetId;
			int16_t ValueSizeDelta;
			std::vector<uint8_t> Value;
		};

		template<typename TTraits>
		struct AccountMetadataTraits {
			using TransactionTraits = TTraits;
			using BuilderType = AccountMetadataBuilder;

			static constexpr auto Entity_Type = model::Entity_Type_Account_Metadata;

			static void SetTargetId(BuilderType&, uint64_t)
			{}

			template<typename TTransaction>
			static void AssertAdditionalProperties(const TransactionProperties&, const TTransaction&)
			{}
		};

		template<typename TTraits>
		struct MosaicMetadataTraits {
			using TransactionTraits = TTraits;
			using BuilderType = MosaicMetadataBuilder;

			static constexpr auto Entity_Type = model::Entity_Type_Mosaic_Metadata;

			static void SetTargetId(BuilderType& builder, uint64_t rawTargetId) {
				builder.setTargetMosaicId(UnresolvedMosaicId(rawTargetId));
			}

			template<typename TTransaction>
			static void AssertAdditionalProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
				EXPECT_EQ(UnresolvedMosaicId(expectedProperties.RawTargetId), transaction.TargetMosaicId);
			}
		};

		template<typename TTraits>
		struct NamespaceMetadataTraits {
			using TransactionTraits = TTraits;
			using BuilderType = NamespaceMetadataBuilder;

			static constexpr auto Entity_Type = model::Entity_Type_Namespace_Metadata;

			static void SetTargetId(BuilderType& builder, uint64_t rawTargetId) {
				builder.setTargetNamespaceId(NamespaceId(rawTargetId));
			}

			template<typename TTransaction>
			static void AssertAdditionalProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
				EXPECT_EQ(NamespaceId(expectedProperties.RawTargetId), transaction.TargetNamespaceId);
			}
		};

		using AccountRegularTraits = AccountMetadataTraits<test::RegularTransactionTraits<model::AccountMetadataTransaction>>;
		using AccountEmbeddedTraits = AccountMetadataTraits<test::EmbeddedTransactionTraits<model::EmbeddedAccountMetadataTransaction>>;
		using MosaicRegularTraits = MosaicMetadataTraits<test::RegularTransactionTraits<model::MosaicMetadataTransaction>>;
		using MosaicEmbeddedTraits = MosaicMetadataTraits<test::EmbeddedTransactionTraits<model::EmbeddedMosaicMetadataTransaction>>;
		using NamespaceRegularTraits = NamespaceMetadataTraits<test::RegularTransactionTraits<model::NamespaceMetadataTransaction>>;
		using NamespaceEmbeddedTraits = NamespaceMetadataTraits<test::EmbeddedTransactionTraits<
			model::EmbeddedNamespaceMetadataTransaction>>;

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.TargetAddress, transaction.TargetAddress);
			EXPECT_EQ(expectedProperties.ScopedMetadataKey, transaction.ScopedMetadataKey);
			EXPECT_EQ(expectedProperties.ValueSizeDelta, transaction.ValueSizeDelta);
			ASSERT_EQ(expectedProperties.Value.size(), transaction.ValueSize);
			EXPECT_EQ_MEMORY(expectedProperties.Value.data(), transaction.ValuePtr(), transaction.ValueSize);
		}

		template<typename TTraits>
		void AssertCanBuildTransaction(
				const TransactionProperties& expectedProperties,
				const consumer<typename TTraits::BuilderType&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			typename TTraits::BuilderType builder(networkIdentifier, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::TransactionTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::TransactionTraits::CheckBuilderSize(expectedProperties.Value.size(), builder);
			TTraits::TransactionTraits::CheckFields(expectedProperties.Value.size(), *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(1u, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
			EXPECT_EQ(TTraits::Entity_Type, pTransaction->Type);

			AssertTransactionProperties(expectedProperties, *pTransaction);
			TTraits::AssertAdditionalProperties(expectedProperties, *pTransaction);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Account_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Account_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountEmbeddedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicEmbeddedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Namespace_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NamespaceRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Namespace_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NamespaceEmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransactionWithDefaultValues) {
		AssertCanBuildTransaction<TTraits>(TransactionProperties(), [](const auto&) {});
	}

	// endregion

	// region additional transaction fields

	TRAITS_BASED_TEST(CanSetTargetAddress) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		test::FillWithRandomData(expectedProperties.TargetAddress);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [&targetAddress = expectedProperties.TargetAddress](auto& builder) {
			builder.setTargetAddress(targetAddress);
		});
	}

	TRAITS_BASED_TEST(CanSetScopedMetadataKey) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.ScopedMetadataKey = test::Random();

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [scopedMetadataKey = expectedProperties.ScopedMetadataKey](auto& builder) {
			builder.setScopedMetadataKey(scopedMetadataKey);
		});
	}

	TRAITS_BASED_TEST(CanSetTargetId) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.RawTargetId = test::Random();

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [rawTargetId = expectedProperties.RawTargetId](auto& builder) {
			TTraits::SetTargetId(builder, rawTargetId);
		});
	}

	TRAITS_BASED_TEST(CanSetValueSizeDelta) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.ValueSizeDelta = static_cast<int16_t>(test::Random());

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [valueSizeDelta = expectedProperties.ValueSizeDelta](auto& builder) {
			builder.setValueSizeDelta(valueSizeDelta);
		});
	}

	TRAITS_BASED_TEST(CanSetValue) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Value = test::GenerateRandomVector(23);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [&value = expectedProperties.Value](auto& builder) {
			builder.setValue(value);
		});
	}

	// endregion
}}
