#include "src/HashLockMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/lock/src/model/HashLockTransaction.h"
#include "mongo/tests/test/MongoTransactionPluginTestUtils.h"
#include "tests/test/LockMapperTestUtils.h"
#include "tests/test/LockTransactionUtils.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS HashLockMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS(HashLock);

		template<typename TTransaction>
		void AssertHashLockTransaction(const TTransaction& transaction, const bsoncxx::document::view& dbTransaction) {
			EXPECT_EQ(transaction.Hash, test::GetHashValue(dbTransaction, "hash"));
		}
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, model::Entity_Type_Hash_Lock)

	// region streamTransaction

	PLUGIN_TEST(CanMapHashLockTransaction) {
		// Arrange:
		auto pTransaction = test::CreateTransaction<TTraits>();
		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, *pTransaction);
		auto view = builder.view();

		// Assert:
		EXPECT_EQ(4u, test::GetFieldCount(view));
		test::AssertEqualNonInheritedLockTransactionData(*pTransaction, view);
		AssertHashLockTransaction(*pTransaction, view);
	}

	// endregion
}}}
