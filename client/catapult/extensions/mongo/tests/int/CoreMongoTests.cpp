#include "mongo/src/CoreMongo.h"
#include "mongo/src/MongoPluginManager.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS CoreSystemTests

	TEST(TEST_CLASS, AppropriateStoragesAreRegistered) {
		// Arrange:
		test::PrepareDatabase(test::DatabaseName());
		MongoStorageContext mongoContext(test::DefaultDbUri(), test::DatabaseName(), nullptr);
		MongoPluginManager manager(mongoContext, model::BlockChainConfiguration::Uninitialized());

		// Act:
		RegisterCoreMongoSystem(manager);

		// Assert:
		auto pStorage = manager.createStorage();
		EXPECT_EQ("{ AccountStateCache, BlockDifficultyCache }", pStorage->name());
	}
}}
