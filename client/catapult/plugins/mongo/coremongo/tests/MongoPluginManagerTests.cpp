#include "src/MongoPluginManager.h"
#include "src/MongoTransactionPlugin.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/mongo/MongoTestUtils.h"
#include "tests/test/mongo/mocks/MockExternalCacheStorage.h"
#include "tests/test/mongo/mocks/MockTransactionMapper.h"
#include "tests/TestHarness.h"
#include <mongocxx/instance.hpp>

#define TEST_CLASS MongoPluginManagerTests

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		template<typename TAction>
		void RunPluginManagerTest(const model::BlockChainConfiguration& config, TAction action) {
			// Arrange:
			// - windows requires the caller to explicitly create a mongocxx instance before certain operations
			//   like creating a mongocxx::pool (via MongoStorageConfiguration)
			mongocxx::instance::current();
			MongoStorageConfiguration mongoConfig(test::DefaultDbUri(), "", nullptr);

			MongoPluginManager manager(mongoConfig, config);

			// Act + Assert:
			action(manager, mongoConfig);
		}

		template<typename TAction>
		void RunPluginManagerTest(TAction action) {
			RunPluginManagerTest(model::BlockChainConfiguration::Uninitialized(), [action](auto& manager, const auto&) {
				action(manager);
			});
		}
	}

	// region basic

	TEST(TEST_CLASS, CanCreateManager) {
		// Arrange:
		auto config = model::BlockChainConfiguration::Uninitialized();
		config.BlockPruneInterval = 15;
		RunPluginManagerTest(config, [](const auto& manager, const auto& mongoConfig) {
			// Assert: compare BlockPruneInterval as a sentinel value because the manager copies the config
			EXPECT_EQ(&mongoConfig, &manager.mongoConfig());
			EXPECT_EQ(15u, manager.chainConfig().BlockPruneInterval);
		});
	}

	// endregion

	// region tx plugins

	TEST(TEST_CLASS, CanRegisterCustomTransactions) {
		// Arrange:
		RunPluginManagerTest([](auto& manager) {
			// Act:
			for (auto i : { 7, 9, 4 })
				manager.addTransactionSupport(mocks::CreateMockTransactionMongoPlugin(i));

			// Assert:
			EXPECT_EQ(3u, manager.transactionRegistry().size());

			for (auto i : { 7, 9, 4 }) {
				auto entityType = static_cast<model::EntityType>(i);
				EXPECT_TRUE(!!manager.transactionRegistry().findPlugin(entityType)) << "type " << i;
			}
		});
	}

	// endregion

	// region external storage plugins

	TEST(TEST_CLASS, CanBuildStorageWithMultipleSubStorages) {
		// Arrange:
		RunPluginManagerTest([](auto& manager) {
			// Act:
			manager.addStorageSupport(std::make_unique<mocks::MockExternalCacheStorage<3>>());
			manager.addStorageSupport(std::make_unique<mocks::MockExternalCacheStorage<6>>());
			manager.addStorageSupport(std::make_unique<mocks::MockExternalCacheStorage<4>>());
			auto pStorage = manager.createStorage();

			// Assert:
			EXPECT_EQ("{ SimpleCache, SimpleCache, SimpleCache }", pStorage->name());
		});
	}

	// endregion
}}}
