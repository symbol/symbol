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

#include "src/builders/MosaicDefinitionBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS MosaicDefinitionBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::MosaicDefinitionTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedMosaicDefinitionTransaction>;

		struct TransactionProperties {
		public:
			TransactionProperties()
					: NamespaceId(test::GenerateRandomValue<catapult::NamespaceId>())
					, MosaicName(test::GenerateRandomString(10))
					, Flags(model::MosaicFlags::None)
					, Divisibility(0)
			{}

		public:
			catapult::NamespaceId NamespaceId;
			std::string MosaicName;
			model::MosaicFlags Flags;
			uint8_t Divisibility;
			std::vector<uint64_t> OptionalValues;
		};

		template<typename TTransaction>
		void AssertMosaicDefinitionName(const TTransaction& transaction, NamespaceId parentId, const std::string& mosaicName) {
			// Assert:
			EXPECT_EQ(mosaicName.size(), transaction.MosaicNameSize);
			EXPECT_TRUE(0 == std::memcmp(mosaicName.data(), transaction.NamePtr(), mosaicName.size()));

			// - id matches
			auto expectedMosaicId = model::GenerateMosaicId(parentId, mosaicName);
			EXPECT_EQ(expectedMosaicId, transaction.MosaicId);
		}

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.Flags, transaction.PropertiesHeader.Flags);
			EXPECT_EQ(expectedProperties.Divisibility, transaction.PropertiesHeader.Divisibility);
			AssertMosaicDefinitionName(transaction, expectedProperties.NamespaceId, expectedProperties.MosaicName);

			// - optional values
			auto expectedPropertyId = model::First_Optional_Property;
			for (auto optionalValue : expectedProperties.OptionalValues) {
				const auto& property = *transaction.PropertiesPtr();
				EXPECT_EQ(property.Id, static_cast<model::MosaicPropertyId>(expectedPropertyId));
				EXPECT_EQ(property.Value, optionalValue);
				++expectedPropertyId;
			}
		}

		template<typename TTraits>
		void AssertCanBuildTransaction(
				size_t propertiesSize,
				const TransactionProperties& expectedProperties,
				const consumer<MosaicDefinitionBuilder&>& buildTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			MosaicDefinitionBuilder builder(networkId, signer, expectedProperties.NamespaceId, expectedProperties.MosaicName);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(expectedProperties.MosaicName.size() + propertiesSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6202, pTransaction->Version);
			EXPECT_EQ(model::Entity_Type_Mosaic_Definition, pTransaction->Type);
			EXPECT_EQ(expectedProperties.NamespaceId, pTransaction->ParentId);

			AssertTransactionProperties(expectedProperties, *pTransaction);
		}

		MosaicDefinitionBuilder CreateBuilderWithName(const RawString& name, const Key& signer) {
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			return MosaicDefinitionBuilder(networkIdentifier, signer, test::GenerateRandomValue<NamespaceId>(), name);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransaction) {
		// Assert:
		AssertCanBuildTransaction<TTraits>(0, TransactionProperties(), [](const auto&) {});
	}

	// endregion

	// region name validation

	TEST(TEST_CLASS, CannotSetEmptyName) {
		// Act + Assert:
		EXPECT_THROW(CreateBuilderWithName({}, test::GenerateRandomData<Key_Size>()), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannnotSetTooLongName) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(1 << (sizeof(model::MosaicDefinitionTransaction::MosaicNameSize) * 8));
		auto signer = test::GenerateRandomData<Key_Size>();
		auto builder = CreateBuilderWithName(namespaceName, signer);

		// Act + Assert:
		EXPECT_THROW(builder.build(), catapult_runtime_error);
	}

	// endregion

	// region required properties

	namespace {
		template<typename TTraits>
		void AssertCanSetFlags(model::MosaicFlags expectedFlags, const consumer<MosaicDefinitionBuilder&>& buildTransaction) {
			// Arrange:
			auto expectedProperties = TransactionProperties();
			expectedProperties.Flags = expectedFlags;

			// Assert:
			AssertCanBuildTransaction<TTraits>(0, expectedProperties, buildTransaction);
		}
	}

	TRAITS_BASED_TEST(CanSetFlags_SupplyMutable) {
		// Assert:
		AssertCanSetFlags<TTraits>(model::MosaicFlags::Supply_Mutable, [](auto& builder) {
			builder.setSupplyMutable();
		});
	}

	TRAITS_BASED_TEST(CanSetFlags_Transferable) {
		// Assert:
		AssertCanSetFlags<TTraits>(model::MosaicFlags::Transferable, [](auto& builder) {
			builder.setTransferable();
		});
	}

	TRAITS_BASED_TEST(CanSetFlags_LevyMutable) {
		// Assert:
		AssertCanSetFlags<TTraits>(model::MosaicFlags::Levy_Mutable, [](auto& builder) {
			builder.setLevyMutable();
		});
	}

	TRAITS_BASED_TEST(CanSetFlags_All) {
		// Assert:
		auto flags = model::MosaicFlags::Supply_Mutable | model::MosaicFlags::Transferable | model::MosaicFlags::Levy_Mutable;
		AssertCanSetFlags<TTraits>(flags, [](auto& builder) {
			builder.setSupplyMutable();
			builder.setLevyMutable();
			builder.setTransferable();
		});
	}

	TRAITS_BASED_TEST(CanSetDivisibility) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Divisibility = 0xA5;

		// Assert:
		AssertCanBuildTransaction<TTraits>(0, expectedProperties, [](auto& builder) {
			builder.setDivisibility(0xA5);
		});
	}

	// endregion

	// region optional properties

	TRAITS_BASED_TEST(CanSetOptionalProperty_Duration) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.OptionalValues = { 12345678 };

		// Assert:
		AssertCanBuildTransaction<TTraits>(sizeof(model::MosaicProperty), expectedProperties, [](auto& builder) {
			builder.setDuration(BlockDuration(12345678));
		});
	}

	// endregion
}}
