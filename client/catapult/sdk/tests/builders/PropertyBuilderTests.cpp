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

#include "src/builders/PropertyBuilder.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS PropertyBuilderTests

	namespace {
		template<typename TModification>
		struct TransactionPropertiesT {
		public:
			TransactionPropertiesT() : PropertyType(model::PropertyType(0))
			{}

		public:
			model::PropertyType PropertyType;
			std::vector<TModification> Modifications;
		};

		// region traits

		template<typename TPropertyValue>
		struct ModificationTraitsMixin {
			using Modification = model::PropertyModification<TPropertyValue>;
			using Modifications = std::vector<Modification>;
			using TransactionProperties = TransactionPropertiesT<Modification>;

			static constexpr size_t Property_Modification_Size = 1 + sizeof(TPropertyValue);
		};

		struct AddressTraits : public ModificationTraitsMixin<UnresolvedAddress> {
			using BuilderType = AddressPropertyBuilder;
			using RegularTraits = test::RegularTransactionTraits<model::AddressPropertyTransaction>;
			using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedAddressPropertyTransaction>;

			static auto TransactionType() {
				return model::Entity_Type_Address_Property;
			}

			static auto RandomUnresolvedValue() {
				return test::GenerateRandomUnresolvedAddress();
			}
		};

		struct MosaicTraits : public ModificationTraitsMixin<UnresolvedMosaicId> {
			using BuilderType = MosaicPropertyBuilder;
			using RegularTraits = test::RegularTransactionTraits<model::MosaicPropertyTransaction>;
			using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedMosaicPropertyTransaction>;

			static auto TransactionType() {
				return model::Entity_Type_Mosaic_Property;
			}

			static auto RandomUnresolvedValue() {
				return test::GenerateRandomValue<UnresolvedMosaicId>();
			}
		};

		struct TransactionTypeTraits : public ModificationTraitsMixin<model::EntityType> {
			using BuilderType = TransactionTypePropertyBuilder;
			using RegularTraits = test::RegularTransactionTraits<model::TransactionTypePropertyTransaction>;
			using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedTransactionTypePropertyTransaction>;

			static auto TransactionType() {
				return model::Entity_Type_Transaction_Type_Property;
			}

			static auto RandomUnresolvedValue() {
				return static_cast<model::EntityType>(test::RandomByte());
			}
		};

		// endregion

		template<typename TTransactionProperties, typename TTransaction>
		void AssertTransactionProperties(const TTransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.PropertyType, transaction.PropertyType);

			auto i = 0u;
			const auto* pModification = transaction.ModificationsPtr();
			for (const auto& modification : expectedProperties.Modifications) {
				auto rawType = static_cast<uint16_t>(modification.ModificationType);
				EXPECT_EQ(modification.ModificationType, pModification->ModificationType) << "type " << rawType << " at index " << i;
				EXPECT_EQ(modification.Value, pModification->Value) << "at index " << i;
				++pModification;
			}
		}

		template<typename TTraits, typename TPropertyTraits>
		void AssertCanBuildTransaction(
				size_t additionalSize,
				const typename TPropertyTraits::TransactionProperties& expectedProperties,
				const consumer<typename TPropertyTraits::BuilderType&>& buildTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			typename TPropertyTraits::BuilderType builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(additionalSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6201, pTransaction->Version);
			EXPECT_EQ(TPropertyTraits::TransactionType(), pTransaction->Type);

			AssertTransactionProperties(expectedProperties, *pTransaction);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits, typename TPropertyTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Address) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename AddressTraits::RegularTraits, AddressTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Address) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename AddressTraits::EmbeddedTraits, AddressTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Mosaic) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename MosaicTraits::RegularTraits, MosaicTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Mosaic) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename MosaicTraits::EmbeddedTraits, MosaicTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Regular_TransactionType) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename TransactionTypeTraits::RegularTraits, TransactionTypeTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_TransactionType) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<typename TransactionTypeTraits::EmbeddedTraits, TransactionTypeTraits>(); \
	} \
	template<typename TTraits, typename TPropertyTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransactionWithDefaultValues) {
		// Assert:
		AssertCanBuildTransaction<TTraits, TPropertyTraits>(0, typename TPropertyTraits::TransactionProperties(), [](const auto&) {});
	}

	// endregion

	// region property type

	TRAITS_BASED_TEST(CanSetPropertyType) {
		// Arrange:
		auto expectedProperties = typename TPropertyTraits::TransactionProperties();
		expectedProperties.PropertyType = model::PropertyType(12);

		// Assert:
		AssertCanBuildTransaction<TTraits, TPropertyTraits>(0, expectedProperties, [](auto& builder) {
			builder.setPropertyType(model::PropertyType(12));
		});
	}

	// endregion

	// region modifications

	namespace {
		template<typename TPropertyTraits>
		auto CreateModifications(uint8_t count) {
			typename TPropertyTraits::Modifications modifications;
			for (auto i = 0u; i < count; ++i) {
				auto type = 0 == i % 2 ? model::PropertyModificationType::Add : model::PropertyModificationType::Del;
				modifications.push_back({ type, TPropertyTraits::RandomUnresolvedValue() });
			}

			return modifications;
		}

		template<typename TPropertyTraits>
		void AddAll(typename TPropertyTraits::BuilderType& builder, const typename TPropertyTraits::Modifications& modifications) {
			for (const auto& modification : modifications)
				builder.addPropertyModification(modification.ModificationType, modification.Value);
		}
	}

	TRAITS_BASED_TEST(CanAddSingleModification) {
		// Arrange:
		auto expectedProperties = typename TPropertyTraits::TransactionProperties();
		expectedProperties.Modifications = CreateModifications<TPropertyTraits>(1);
		const auto& modifications = expectedProperties.Modifications;

		// Assert:
		auto totalCosignatorySize = TPropertyTraits::Property_Modification_Size;
		AssertCanBuildTransaction<TTraits, TPropertyTraits>(totalCosignatorySize, expectedProperties, [&modifications](auto& builder) {
			AddAll<TPropertyTraits>(builder, modifications);
		});
	}

	TRAITS_BASED_TEST(CanAddMultipleModifications) {
		// Arrange:
		auto expectedProperties = typename TPropertyTraits::TransactionProperties();
		expectedProperties.Modifications = CreateModifications<TPropertyTraits>(5);
		const auto& modifications = expectedProperties.Modifications;

		// Assert:
		auto totalCosignatorySize = 5 * TPropertyTraits::Property_Modification_Size;
		AssertCanBuildTransaction<TTraits, TPropertyTraits>(totalCosignatorySize, expectedProperties, [&modifications](auto& builder) {
			AddAll<TPropertyTraits>(builder, modifications);
		});
	}

	TRAITS_BASED_TEST(CanSetPropertyTypeAndAddModifications) {
		// Arrange:
		auto expectedProperties = typename TPropertyTraits::TransactionProperties();
		expectedProperties.PropertyType = model::PropertyType(12);
		expectedProperties.Modifications = CreateModifications<TPropertyTraits>(4);
		const auto& modifications = expectedProperties.Modifications;

		// Assert:
		auto totalCosignatorySize = 4 * TPropertyTraits::Property_Modification_Size;
		AssertCanBuildTransaction<TTraits, TPropertyTraits>(totalCosignatorySize, expectedProperties, [&modifications](auto& builder) {
			builder.setPropertyType(model::PropertyType(12));
			AddAll<TPropertyTraits>(builder, modifications);
		});
	}

	// endregion
}}
