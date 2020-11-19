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

#include "src/builders/MosaicGlobalRestrictionBuilder.h"
#include "tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS MosaicGlobalRestrictionBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::MosaicGlobalRestrictionTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedMosaicGlobalRestrictionTransaction>;

		struct TransactionProperties {
		public:
			TransactionProperties()
					: RestrictionKey()
					, PreviousRestrictionValue()
					, PreviousRestrictionType()
					, NewRestrictionValue()
					, NewRestrictionType()
			{}

		public:
			UnresolvedMosaicId MosaicId;
			UnresolvedMosaicId ReferenceMosaicId;
			uint64_t RestrictionKey;
			uint64_t PreviousRestrictionValue;
			model::MosaicRestrictionType PreviousRestrictionType;
			uint64_t NewRestrictionValue;
			model::MosaicRestrictionType NewRestrictionType;
		};

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.MosaicId, transaction.MosaicId);
			EXPECT_EQ(expectedProperties.ReferenceMosaicId, transaction.ReferenceMosaicId);
			EXPECT_EQ(expectedProperties.RestrictionKey, transaction.RestrictionKey);
			EXPECT_EQ(expectedProperties.PreviousRestrictionValue, transaction.PreviousRestrictionValue);
			EXPECT_EQ(expectedProperties.PreviousRestrictionType, transaction.PreviousRestrictionType);
			EXPECT_EQ(expectedProperties.NewRestrictionValue, transaction.NewRestrictionValue);
			EXPECT_EQ(expectedProperties.NewRestrictionType, transaction.NewRestrictionType);
		}

		template<typename TTraits>
		void AssertCanBuildTransaction(
				const TransactionProperties& expectedProperties,
				const consumer<MosaicGlobalRestrictionBuilder&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			MosaicGlobalRestrictionBuilder builder(networkIdentifier, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckBuilderSize(0, builder);
			TTraits::CheckFields(0, *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(1u, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
			EXPECT_EQ(model::Entity_Type_Mosaic_Global_Restriction, pTransaction->Type);

			AssertTransactionProperties(expectedProperties, *pTransaction);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransactionWithDefaultValues) {
		AssertCanBuildTransaction<TTraits>(TransactionProperties(), [](const auto&) {});
	}

	// endregion

	// region additional transaction fields

	TRAITS_BASED_TEST(CanSetMosaicId) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.MosaicId = UnresolvedMosaicId(123);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setMosaicId(UnresolvedMosaicId(123));
		});
	}

	TRAITS_BASED_TEST(CanSetReferenceMosaicId) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.ReferenceMosaicId = UnresolvedMosaicId(345);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setReferenceMosaicId(UnresolvedMosaicId(345));
		});
	}

	TRAITS_BASED_TEST(CanSetRestrictionKey) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.RestrictionKey = test::Random();

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [restrictionKey = expectedProperties.RestrictionKey](auto& builder) {
			builder.setRestrictionKey(restrictionKey);
		});
	}

	TRAITS_BASED_TEST(CanSetOtherProperties) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.PreviousRestrictionValue = 1234;
		expectedProperties.PreviousRestrictionType = static_cast<model::MosaicRestrictionType>(0x56);
		expectedProperties.NewRestrictionValue = 7890;
		expectedProperties.NewRestrictionType = static_cast<model::MosaicRestrictionType>(0xAB);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setPreviousRestrictionValue(1234);
			builder.setPreviousRestrictionType(static_cast<model::MosaicRestrictionType>(0x56));
			builder.setNewRestrictionValue(7890);
			builder.setNewRestrictionType(static_cast<model::MosaicRestrictionType>(0xAB));
		});
	}

	// endregion
}}
