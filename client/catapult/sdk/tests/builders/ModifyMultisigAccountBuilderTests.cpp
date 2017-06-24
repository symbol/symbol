#include "src/builders/ModifyMultisigAccountBuilder.h"
#include "tests/TestHarness.h"

#define TEST_CLASS ModifyMultisigAccountBuilderTests

namespace catapult { namespace builders {

	namespace {
		void AssertCanBuildTransaction(
				size_t additionalSize,
				const std::function<void (ModifyMultisigAccountBuilder&)>& buildTransaction,
				const std::function<void (const model::ModifyMultisigAccountTransaction&)>& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			ModifyMultisigAccountBuilder builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = builder.build();

			// Assert:
			ASSERT_EQ(sizeof(model::ModifyMultisigAccountTransaction) + additionalSize, pTransaction->Size);
			EXPECT_EQ(Signature{}, pTransaction->Signature);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6203, pTransaction->Version);
			EXPECT_EQ(model::EntityType::Modify_Multisig_Account, pTransaction->Type);

			EXPECT_EQ(Amount(0), pTransaction->Fee);
			EXPECT_EQ(Timestamp(0), pTransaction->Deadline);

			validateTransaction(*pTransaction);
		}

		auto CreatePropertyChecker(
				int8_t minRemovalDelta,
				int8_t minApprovalDelta,
				const std::vector<model::CosignatoryModification>& modifications) {
			return [minRemovalDelta, minApprovalDelta, &modifications](const auto& transaction) {
				EXPECT_EQ(minRemovalDelta, transaction.MinRemovalDelta);
				EXPECT_EQ(minApprovalDelta, transaction.MinApprovalDelta);

				auto i = 0u;
				const auto* pModification = transaction.ModificationsPtr();
				for (const auto& modification : modifications) {
					EXPECT_EQ(modification.ModificationType, pModification->ModificationType)
							<< "type " << static_cast<uint16_t>(modification.ModificationType) << "at index " << i;
					EXPECT_EQ(modification.CosignatoryPublicKey, pModification->CosignatoryPublicKey)
							<< " key " << test::ToHexString(modification.CosignatoryPublicKey) << "at index " << i;
					++pModification;
				}
			};
		}
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateTransactionWithDefaultValues) {
		// Assert:
		AssertCanBuildTransaction(0, [](const auto&) {}, CreatePropertyChecker(0, 0, {}));
	}

	// endregion

	// region min cosignatory settings

	TEST(TEST_CLASS, CanSetMinRemovalDelta) {
		// Assert:
		AssertCanBuildTransaction(
				0,
				[](auto& builder) {
					builder.setMinRemovalDelta(3);
				},
				CreatePropertyChecker(3, 0, {}));
	}

	TEST(TEST_CLASS, CanSetMinApprovalDelta) {
		// Assert:
		AssertCanBuildTransaction(
				0,
				[](auto& builder) {
					builder.setMinApprovalDelta(3);
				},
				CreatePropertyChecker(0, 3, {}));
	}

	// endregion

	// region modifications

	namespace {
		auto CreateModifications(uint8_t count) {
			std::vector<model::CosignatoryModification> modifications;
			for (auto i = 0u; i < count; ++i) {
				auto type = 0 == i % 2 ? model::CosignatoryModificationType::Add : model::CosignatoryModificationType::Del;
				modifications.push_back(model::CosignatoryModification{ type, test::GenerateRandomData<Key_Size>() });
			}

			return modifications;
		}

		void AddAll(ModifyMultisigAccountBuilder& builder, const std::vector<model::CosignatoryModification>& modifications) {
			for (const auto& modification : modifications)
				builder.addCosignatoryModification(modification.ModificationType, modification.CosignatoryPublicKey);
		}
	}

	TEST(TEST_CLASS, CanAddSingleModification) {
		// Arrange:
		auto modifications = CreateModifications(1);

		// Assert:
		AssertCanBuildTransaction(
				sizeof(model::CosignatoryModification),
				[&modifications](auto& builder) {
					AddAll(builder, modifications);
				},
				CreatePropertyChecker(0, 0, modifications));
	}

	TEST(TEST_CLASS, CanAddMultipleModifications) {
		// Arrange:
		auto modifications = CreateModifications(5);

		// Assert:
		AssertCanBuildTransaction(
				5 * sizeof(model::CosignatoryModification),
				[&modifications](auto& builder) {
					AddAll(builder, modifications);
				},
				CreatePropertyChecker(0, 0, modifications));
	}

	TEST(TEST_CLASS, CanSetDeltasAndAddModifications) {
		// Arrange:
		auto modifications = CreateModifications(4);

		// Assert:
		AssertCanBuildTransaction(
				4 * sizeof(model::CosignatoryModification),
				[&modifications](auto& builder) {
					builder.setMinRemovalDelta(-3);
					builder.setMinApprovalDelta(3);
					AddAll(builder, modifications);
				},
				CreatePropertyChecker(-3, 3, modifications));
	}

	// endregion
}}
