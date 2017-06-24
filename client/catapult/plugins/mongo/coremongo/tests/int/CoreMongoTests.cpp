#include "src/CoreMongo.h"
#include "src/MongoPluginManager.h"
#include "tests/test/mongo/MongoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		constexpr char Database_Name[] = "test-db";

		auto CreateMongoStorageConfiguration() {
			test::PrepareDatabase(Database_Name);
			return std::make_shared<plugins::MongoStorageConfiguration>(mongocxx::uri(), Database_Name, nullptr, nullptr);
		}
	}

	TEST(CoreSystemTests, AppropriateStoragesAreRegistered) {
		// Arrange:
		MongoPluginManager manager(model::BlockChainConfiguration::Uninitialized());
		manager.setMongoConfig(CreateMongoStorageConfiguration());

		// Act:
		RegisterCoreSystem(manager);

		// Assert:
		auto pStorage = manager.createStorage();
		EXPECT_EQ("{ AccountStateCache, BlockDifficultyCache }", pStorage->name());
	}
}}}
