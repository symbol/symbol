#include "src/ModifyMultisigAccountMapper.h"
#include "sdk/src/builders/ModifyMultisigAccountBuilder.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "catapult/constants.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/test/mongo/MongoTransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

#define TEST_CLASS ModifyMultisigAccountMapperTests

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(ModifyMultisigAccount);

		using ModificationType = model::CosignatoryModificationType;

		auto CreateModifyMultisigAccountTransaction(
				int8_t minRemovalDelta,
				int8_t minApprovalDelta,
				std::initializer_list<model::CosignatoryModification> modifications) {
			auto signer = test::GenerateKeyPair();
			builders::ModifyMultisigAccountBuilder builder(model::NetworkIdentifier::Mijin_Test, signer.publicKey());
			builder.setMinRemovalDelta(minRemovalDelta);
			builder.setMinApprovalDelta(minApprovalDelta);
			for (const auto& modification : modifications)
				builder.addCosignatoryModification(modification.ModificationType, modification.CosignatoryPublicKey);

			return builder.build();
		}

		template<typename TTransaction>
		void AssertEqualNonInheritedTransferData(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
			EXPECT_EQ(transaction.MinRemovalDelta, test::GetUint32(dbTransaction, "minRemovalDelta"));
			EXPECT_EQ(transaction.MinApprovalDelta, test::GetUint32(dbTransaction, "minApprovalDelta"));

			auto dbModifications = dbTransaction["modifications"].get_array().value;
			ASSERT_EQ(transaction.ModificationsCount, std::distance(dbModifications.cbegin(), dbModifications.cend()));
			const auto* pModification = transaction.ModificationsPtr();
			auto iter = dbModifications.cbegin();
			for (auto i = 0u; i < transaction.ModificationsCount; ++i) {
				EXPECT_EQ(
						pModification->ModificationType,
						static_cast<ModificationType>(test::GetUint32(iter->get_document().view(), "type")));
				EXPECT_EQ(
						test::ToHexString(pModification->CosignatoryPublicKey),
						test::ToHexString(test::GetBinary(iter->get_document().view(), "cosignatoryPublicKey"), Key_Size));
				++pModification;
				++iter;
			}
		}

		template<typename TTraits>
		void AssertCanMapModifyMultisigAccountTransaction(
				int8_t minRemovalDelta,
				int8_t minApprovalDelta,
				std::initializer_list<model::CosignatoryModification> modifications) {
			// Arrange:
			auto pTransaction = TTraits::Adapt(CreateModifyMultisigAccountTransaction(minRemovalDelta, minApprovalDelta, modifications));
			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, *pTransaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(3u, test::GetFieldCount(view));
			AssertEqualNonInheritedTransferData(*pTransaction, view);
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, model::EntityType::Modify_Multisig_Account)

	// region streamTransaction

	PLUGIN_TEST(CanMapModifiyMultisigAccountTransactionWithoutModification) {
		// Assert:
		AssertCanMapModifyMultisigAccountTransaction<TTraits>(3, 5, {});
	}

	PLUGIN_TEST(CanMapModifiyMultisigAccountTransactionWithSingleModification) {
		// Assert:
		AssertCanMapModifyMultisigAccountTransaction<TTraits>(3, 5, { { ModificationType::Add, test::GenerateRandomData<Key_Size>() } });
	}

	PLUGIN_TEST(CanMapModifiyMultisigAccountTransactionWithMultipleModification) {
		// Assert:
		AssertCanMapModifyMultisigAccountTransaction<TTraits>(3, 5, {
			{ ModificationType::Add, test::GenerateRandomData<Key_Size>() },
			{ ModificationType::Del, test::GenerateRandomData<Key_Size>() },
			{ ModificationType::Add, test::GenerateRandomData<Key_Size>() },
		});
	}

	// endregion
}}}
