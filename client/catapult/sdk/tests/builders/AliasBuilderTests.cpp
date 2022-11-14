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

#include "src/builders/AddressAliasBuilder.h"
#include "src/builders/MosaicAliasBuilder.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS AliasBuilderTests

	namespace {
		struct AddressAliasTestTraits {
			using AliasedType = Address;

			static constexpr auto Transaction_Type = model::Entity_Type_Alias_Address;

			template<typename TTransaction>
			static Address GetAliased(const TTransaction& transaction) {
				return transaction.Address;
			}

			template<typename TBuilder>
			static void SetAliased(TBuilder& builder, const Address& address) {
				builder.setAddress(address);
			}
		};

		struct MosaicAliasTestTraits {
			using AliasedType = MosaicId;

			static constexpr auto Transaction_Type = model::Entity_Type_Alias_Mosaic;

			template<typename TTransaction>
			static MosaicId GetAliased(const TTransaction& transaction) {
				return transaction.MosaicId;
			}

			template<typename TBuilder>
			static void SetAliased(TBuilder& builder, MosaicId mosaicId) {
				builder.setMosaicId(mosaicId);
			}
		};

		template<typename TBuilder, typename TExpectedTraits>
		struct AliasTraits : public TExpectedTraits {
		public:
			using Builder = TBuilder;
			using RegularTraits = test::RegularTransactionTraits<typename Builder::Transaction>;
			using EmbeddedTraits = test::EmbeddedTransactionTraits<typename Builder::EmbeddedTransaction>;

		public:
			struct TransactionProperties {
			public:
				TransactionProperties(catapult::NamespaceId namespaceId, model::AliasAction aliasAction)
						: NamespaceId(namespaceId)
						, AliasAction(aliasAction)
						, Aliased()
				{}

			public:
				catapult::NamespaceId NamespaceId;
				model::AliasAction AliasAction;
				typename TExpectedTraits::AliasedType Aliased;
			};

		public:
			static void SetAliased(Builder& builder, const typename TExpectedTraits::AliasedType& aliased) {
				TExpectedTraits::SetAliased(builder, aliased);
			}

			template<typename TTransaction>
			static void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
				EXPECT_EQ(expectedProperties.NamespaceId, transaction.NamespaceId);
				EXPECT_EQ(expectedProperties.AliasAction, transaction.AliasAction);
				EXPECT_EQ(expectedProperties.Aliased, TExpectedTraits::GetAliased(transaction));
			}
		};

		using AddressAliasTraits = AliasTraits<AddressAliasBuilder, AddressAliasTestTraits>;
		using MosaicAliasTraits = AliasTraits<MosaicAliasBuilder, MosaicAliasTestTraits>;

		template<typename TAliasTraits, typename TTraits>
		void AssertCanBuildTransaction(
				const typename TAliasTraits::TransactionProperties& expectedProperties,
				const consumer<typename TAliasTraits::Builder&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			auto builder = typename TAliasTraits::Builder(networkIdentifier, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckBuilderSize(0, builder);
			TTraits::CheckFields(0, *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(1u, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
			EXPECT_EQ(TAliasTraits::Transaction_Type, pTransaction->Type);

			TAliasTraits::AssertTransactionProperties(expectedProperties, *pTransaction);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TAliasTraits, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address_Regular) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressAliasTraits, AddressAliasTraits::RegularTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Address_Embedded) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressAliasTraits, AddressAliasTraits::EmbeddedTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Regular) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicAliasTraits, MosaicAliasTraits::RegularTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic_Embedded) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicAliasTraits, MosaicAliasTraits::EmbeddedTraits>(); \
	} \
	template<typename TAliasTraits, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransaction) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto expectedProperties = typename TAliasTraits::TransactionProperties(namespaceId, model::AliasAction::Unlink);

		// Assert:
		AssertCanBuildTransaction<TAliasTraits, TTraits>(expectedProperties, [namespaceId](auto& builder) {
			builder.setNamespaceId(namespaceId);
		});
	}

	// endregion

	// region settings

	TRAITS_BASED_TEST(CanSetAlias) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto expectedProperties = typename TAliasTraits::TransactionProperties(namespaceId, model::AliasAction::Unlink);
		test::FillWithRandomData(expectedProperties.Aliased);
		const auto& aliased = expectedProperties.Aliased;

		// Assert:
		AssertCanBuildTransaction<TAliasTraits, TTraits>(expectedProperties, [namespaceId, &aliased](auto& builder) {
			builder.setNamespaceId(namespaceId);
			TAliasTraits::SetAliased(builder, aliased);
		});
	}

	TRAITS_BASED_TEST(CanSetAction) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto expectedProperties = typename TAliasTraits::TransactionProperties(namespaceId, static_cast<model::AliasAction>(0x45));

		// Assert:
		AssertCanBuildTransaction<TAliasTraits, TTraits>(expectedProperties, [namespaceId](auto& builder) {
			builder.setNamespaceId(namespaceId);
			builder.setAliasAction(static_cast<model::AliasAction>(0x45));
		});
	}

	TRAITS_BASED_TEST(CanSetAliasAndAction) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto aliasAction = static_cast<model::AliasAction>(0x45);
		auto expectedProperties = typename TAliasTraits::TransactionProperties(namespaceId, aliasAction);
		test::FillWithRandomData(expectedProperties.Aliased);
		const auto& aliased = expectedProperties.Aliased;

		// Assert:
		AssertCanBuildTransaction<TAliasTraits, TTraits>(expectedProperties, [aliasAction, namespaceId, &aliased](auto& builder) {
			builder.setNamespaceId(namespaceId);
			TAliasTraits::SetAliased(builder, aliased);
			builder.setAliasAction(aliasAction);
		});
	}

	// endregion
}}
