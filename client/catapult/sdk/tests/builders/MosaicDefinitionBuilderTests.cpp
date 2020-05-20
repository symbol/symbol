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
			BlockDuration Duration;
			catapult::MosaicNonce MosaicNonce;
		};

		template<typename TTransaction>
		void AssertMosaicDefinitionName(const TTransaction& transaction, MosaicNonce nonce) {
			// Assert: id matches
			auto expectedId = model::GenerateMosaicId(model::GetSignerAddress(transaction), nonce);
			EXPECT_EQ(expectedId, transaction.Id);
		}

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.Flags, transaction.Flags);
			EXPECT_EQ(expectedProperties.Divisibility, transaction.Divisibility);
			EXPECT_EQ(expectedProperties.Duration, transaction.Duration);

			AssertMosaicDefinitionName(transaction, expectedProperties.MosaicNonce);
		}

		template<typename TTraits>
		void AssertCanBuildTransaction(
				size_t propertiesSize,
				const TransactionProperties& expectedProperties,
				const consumer<MosaicDefinitionBuilder&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			MosaicDefinitionBuilder builder(networkIdentifier, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckBuilderSize(propertiesSize, builder);
			TTraits::CheckFields(propertiesSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(1u, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
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
		AssertCanBuildTransaction<TTraits>(0, TransactionProperties(), [](const auto&) {});
	}

	// endregion

	// region properties

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

	TRAITS_BASED_TEST(CanSetFlags_Single) {
		AssertCanSetFlags<TTraits>(model::MosaicFlags::Transferable, [](auto& builder) {
			builder.setFlags(model::MosaicFlags::Transferable);
		});
	}

	TRAITS_BASED_TEST(CanSetFlags_All) {
		AssertCanSetFlags<TTraits>(model::MosaicFlags::All, [](auto& builder) {
			builder.setFlags(model::MosaicFlags::All);
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

	TRAITS_BASED_TEST(CanSetDuration) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Duration = BlockDuration(1234);

		// Assert:
		AssertCanBuildTransaction<TTraits>(0, expectedProperties, [](auto& builder) {
			builder.setDuration(BlockDuration(1234));
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
			builder.setNonce(nonce);
		});
	}

	// endregion
}}
