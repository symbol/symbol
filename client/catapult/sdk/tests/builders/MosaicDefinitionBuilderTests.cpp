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
#include "plugins/txes/mosaic/src/model/MosaicIdGenerator.h"
#include "catapult/constants.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS MosaicDefinitionBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::MosaicDefinitionTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedMosaicDefinitionTransaction>;

		struct TransactionProperties {
		public:
			TransactionProperties()
					: Flags(model::MosaicFlags::None)
					, Divisibility(0)
			{}

		public:
			model::MosaicFlags Flags;
			uint8_t Divisibility;
			catapult::MosaicNonce MosaicNonce;
			std::vector<uint64_t> OptionalValues;
		};

		template<typename TTransaction>
		void AssertMosaicDefinitionName(const TTransaction& transaction, MosaicNonce nonce) {
			// Assert: id matches
			auto expectedMosaicId = model::GenerateMosaicId(transaction.Signer, nonce);
			EXPECT_EQ(expectedMosaicId, transaction.MosaicId);
		}

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.Flags, transaction.PropertiesHeader.Flags);
			EXPECT_EQ(expectedProperties.Divisibility, transaction.PropertiesHeader.Divisibility);
			AssertMosaicDefinitionName(transaction, expectedProperties.MosaicNonce);

			// - optional values
			EXPECT_EQ(expectedProperties.OptionalValues.size(), transaction.PropertiesHeader.Count);

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
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			MosaicDefinitionBuilder builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(propertiesSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6203, pTransaction->Version);
			EXPECT_EQ(model::Entity_Type_Mosaic_Definition, pTransaction->Type);

			AssertTransactionProperties(expectedProperties, *pTransaction);
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
			builder.setFlags(model::MosaicFlags::Supply_Mutable);
		});
	}

	TRAITS_BASED_TEST(CanSetFlags_Transferable) {
		// Assert:
		AssertCanSetFlags<TTraits>(model::MosaicFlags::Transferable, [](auto& builder) {
			builder.setFlags(model::MosaicFlags::Transferable);
		});
	}

	TRAITS_BASED_TEST(CanSetFlags_All) {
		// Assert:
		auto flags = model::MosaicFlags::Supply_Mutable | model::MosaicFlags::Transferable;
		AssertCanSetFlags<TTraits>(flags, [](auto& builder) {
			builder.setFlags(model::MosaicFlags::Supply_Mutable | model::MosaicFlags::Transferable);
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
			builder.addProperty({ model::MosaicPropertyId::Duration, 12345678 });
		});
	}

	// endregion

	// region nonce

	TRAITS_BASED_TEST(CanSetNonce) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.MosaicNonce = test::GenerateRandomValue<MosaicNonce>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(0, expectedProperties, [nonce = expectedProperties.MosaicNonce](auto& builder) {
			builder.setMosaicNonce(nonce);
		});
	}

	// endregion
}}
