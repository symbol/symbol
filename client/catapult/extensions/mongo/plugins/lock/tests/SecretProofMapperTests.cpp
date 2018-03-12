#include "src/SecretProofMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock/src/model/SecretProofTransaction.h"
#include "mongo/tests/test/MongoTransactionPluginTestUtils.h"
#include "tests/test/LockMapperTestUtils.h"
#include "tests/test/LockTransactionUtils.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS SecretProofMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(SecretProof);
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, model::Entity_Type_Secret_Proof)

	// region streamTransaction

	PLUGIN_TEST(CanMapSecretProofTransactionWithProof) {
		// Arrange:
		auto pTransaction = test::CreateSecretProofTransaction<TTraits>(123);
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(3u, test::GetFieldCount(view));
		EXPECT_EQ(utils::to_underlying_type(pTransaction->HashAlgorithm), test::GetUint8(view, "hashAlgorithm"));
		EXPECT_EQ(pTransaction->Secret, test::GetHash512Value(view, "secret"));

		const auto* pProof = pTransaction->ProofPtr();
		const auto* pDbProof = test::GetBinary(view, "proof");
		EXPECT_EQ(test::ToHexString(pProof, pTransaction->ProofSize), test::ToHexString(pDbProof, pTransaction->ProofSize));
	}

	// endregion
}}}
