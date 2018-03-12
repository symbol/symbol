#include "src/SecretLockMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock/src/model/SecretLockTransaction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTestUtils.h"
#include "tests/test/LockMapperTestUtils.h"
#include "tests/test/LockTransactionUtils.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS SecretLockMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(SecretLock);

		template<typename TTransaction>
		void AssertSecretLockTransaction(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
			EXPECT_EQ(utils::to_underlying_type(transaction.HashAlgorithm), test::GetUint32(dbTransaction, "hashAlgorithm"));
			EXPECT_EQ(transaction.Secret, test::GetBinaryArray<Hash512_Size>(dbTransaction, "secret"));
			EXPECT_EQ(transaction.Recipient, test::GetAddressValue(dbTransaction, "recipient"));
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, model::Entity_Type_Secret_Lock)

	// region streamTransaction

	PLUGIN_TEST(CanMapSecretLockTransaction) {
		// Arrange:
		auto pTransaction = test::CreateTransaction<TTraits>();
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(6u, test::GetFieldCount(view));
		test::AssertEqualNonInheritedLockTransactionData(*pTransaction, view);
		AssertSecretLockTransaction(*pTransaction, view);
	}

	// endregion
}}}
