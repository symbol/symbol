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

#include "src/builders/MosaicSupplyChangeBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS MosaicSupplyChangeBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::MosaicSupplyChangeTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedMosaicSupplyChangeTransaction>;

		using TransactionType = model::MosaicSupplyChangeTransaction;
		using TransactionPtr = std::unique_ptr<TransactionType>;

		auto CreateTransactionBuilder(MosaicId mosaicId, const consumer<MosaicSupplyChangeBuilder&>& prepareTransaction) {
			return [mosaicId, prepareTransaction](model::NetworkIdentifier networkId, const Key& signer) {
				MosaicSupplyChangeBuilder builder(networkId, signer, mosaicId);
				prepareTransaction(builder);
				return builder;
			};
		}

		auto CreateTransactionBuilder(
				NamespaceId namespaceId,
				const std::string& mosaicName,
				const consumer<MosaicSupplyChangeBuilder&>& prepareTransaction) {
			return [namespaceId, mosaicName, prepareTransaction](model::NetworkIdentifier networkId, const Key& signer) {
				MosaicSupplyChangeBuilder builder(networkId, signer, namespaceId, mosaicName);
				prepareTransaction(builder);
				return builder;
			};
		}

		template<typename TTraits, typename TValidationFunction>
		void AssertCanBuildTransaction(
				const std::function<MosaicSupplyChangeBuilder (model::NetworkIdentifier, const Key&)>& builderFactory,
				const TValidationFunction& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			auto builder = builderFactory(networkId, signer);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(0, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6202, pTransaction->Version);
			EXPECT_EQ(model::Entity_Type_Mosaic_Supply_Change, pTransaction->Type);

			validateTransaction(*pTransaction);
		}

		auto CreatePropertyChecker(MosaicId mosaicId, model::MosaicSupplyChangeDirection direction, Amount delta) {
			return [mosaicId, direction, delta](const auto& transaction) {
				EXPECT_EQ(mosaicId, transaction.MosaicId);
				EXPECT_EQ(direction, transaction.Direction);
				EXPECT_EQ(delta, transaction.Delta);
			};
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransaction) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				CreateTransactionBuilder(mosaicId, [](const auto&) {}),
				CreatePropertyChecker(mosaicId, model::MosaicSupplyChangeDirection::Increase, Amount(0)));
	}

	TRAITS_BASED_TEST(CanCreateTransactionUsingNamespaceIdAndName) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto mosaicName = std::string();

		// Assert:
		auto mosaicId = model::GenerateMosaicId(namespaceId, mosaicName);
		AssertCanBuildTransaction<TTraits>(
				CreateTransactionBuilder(namespaceId, mosaicName, [](const auto&) {}),
				CreatePropertyChecker(mosaicId, model::MosaicSupplyChangeDirection::Increase, Amount(0)));
	}

	// endregion

	// region settings

	TRAITS_BASED_TEST(CanSetDecrease) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				CreateTransactionBuilder(mosaicId, [](auto& builder) {
					builder.setDecrease();
				}),
				CreatePropertyChecker(mosaicId, model::MosaicSupplyChangeDirection::Decrease, Amount(0)));
	}

	TRAITS_BASED_TEST(CanSetDelta) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				CreateTransactionBuilder(mosaicId, [](auto& builder) {
					builder.setDelta(Amount(12345678));
				}),
				CreatePropertyChecker(mosaicId, model::MosaicSupplyChangeDirection::Increase, Amount(12345678)));
	}

	TRAITS_BASED_TEST(CanSetDecreaseAndDelta) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				CreateTransactionBuilder(mosaicId, [](auto& builder) {
					builder.setDecrease();
					builder.setDelta(Amount(12345678));
				}),
				CreatePropertyChecker(mosaicId, model::MosaicSupplyChangeDirection::Decrease, Amount(12345678)));
	}

	// endregion
}}
