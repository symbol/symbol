#include "src/builders/MosaicSupplyChangeBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"
#include "tests/TestHarness.h"

#define TEST_CLASS MosaicSupplyChangeBuilderTests

namespace catapult { namespace builders {

	namespace {
		using TransactionType = model::MosaicSupplyChangeTransaction;
		using TransactionPtr = std::unique_ptr<TransactionType>;

		auto CreateTransactionBuilder(MosaicId mosaicId, const std::function<void (MosaicSupplyChangeBuilder&)>& prepareTransaction) {
			return [mosaicId, prepareTransaction](model::NetworkIdentifier networkId, const Key& signer) {
				MosaicSupplyChangeBuilder builder(networkId, signer, mosaicId);
				prepareTransaction(builder);
				return builder.build();
			};
		}

		auto CreateTransactionBuilder(
				NamespaceId namespaceId,
				const std::string& mosaicName,
				const std::function<void (MosaicSupplyChangeBuilder&)>& prepareTransaction) {
			return [namespaceId, mosaicName, prepareTransaction](model::NetworkIdentifier networkId, const Key& signer) {
				MosaicSupplyChangeBuilder builder(networkId, signer, namespaceId, mosaicName);
				prepareTransaction(builder);
				return builder.build();
			};
		}

		void AssertCanBuildTransaction(
				const std::function<TransactionPtr (model::NetworkIdentifier, const Key&)>& buildTransaction,
				const std::function<void (const model::MosaicSupplyChangeTransaction&)>& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			auto pTransaction = buildTransaction(networkId, signer);

			// Assert:
			ASSERT_EQ(sizeof(model::MosaicSupplyChangeTransaction), pTransaction->Size);
			EXPECT_EQ(Signature{}, pTransaction->Signature);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6202, pTransaction->Version);
			EXPECT_EQ(model::EntityType::Mosaic_Supply_Change, pTransaction->Type);

			EXPECT_EQ(Amount(0), pTransaction->Fee);
			EXPECT_EQ(Timestamp(0), pTransaction->Deadline);

			validateTransaction(*pTransaction);
		}

		auto CreatePropertyChecker(
				MosaicId mosaicId,
				model::MosaicSupplyChangeDirection direction,
				Amount delta) {
			return [mosaicId, direction, delta](const auto& transaction) {
				EXPECT_EQ(mosaicId, transaction.MosaicId);
				EXPECT_EQ(direction, transaction.Direction);
				EXPECT_EQ(delta, transaction.Delta);
			};
		}
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateTransaction) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();

		// Assert:
		AssertCanBuildTransaction(
				CreateTransactionBuilder(mosaicId, [](const auto&) {}),
				CreatePropertyChecker(mosaicId, model::MosaicSupplyChangeDirection::Increase, Amount(0)));
	}

	TEST(TEST_CLASS, CanCreateTransactionUsingNamespaceIdAndName) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto mosaicName = std::string();

		// Assert:
		auto mosaicId = model::GenerateMosaicId(namespaceId, mosaicName);
		AssertCanBuildTransaction(
				CreateTransactionBuilder(namespaceId, mosaicName, [](const auto&) {}),
				CreatePropertyChecker(mosaicId, model::MosaicSupplyChangeDirection::Increase, Amount(0)));
	}

	// endregion

	// region settings

	TEST(TEST_CLASS, CanSetDecrease) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();

		// Assert:
		AssertCanBuildTransaction(
				CreateTransactionBuilder(mosaicId, [](auto& builder) {
					builder.setDecrease();
				}),
				CreatePropertyChecker(mosaicId, model::MosaicSupplyChangeDirection::Decrease, Amount(0)));
	}

	TEST(TEST_CLASS, CanSetDelta) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();

		// Assert:
		AssertCanBuildTransaction(
				CreateTransactionBuilder(mosaicId, [](auto& builder) {
					builder.setDelta(Amount(12345678));
				}),
				CreatePropertyChecker(mosaicId, model::MosaicSupplyChangeDirection::Increase, Amount(12345678)));
	}

	TEST(TEST_CLASS, CanSetDecreaseAndDelta) {
		// Arrange:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();

		// Assert:
		AssertCanBuildTransaction(
				CreateTransactionBuilder(mosaicId, [](auto& builder) {
					builder.setDecrease();
					builder.setDelta(Amount(12345678));
				}),
				CreatePropertyChecker(mosaicId, model::MosaicSupplyChangeDirection::Decrease, Amount(12345678)));
	}

	// endregion
}}
