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

#include "mongo/src/MongoPluginManager.h"
#include "mongo/src/MongoTransactionPlugin.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "mongo/tests/test/mocks/MockTransactionMapper.h"
#include "tests/test/local/mocks/MockExternalCacheStorage.h"
#include "tests/TestHarness.h"
#include <mongocxx/instance.hpp>

namespace catapult { namespace mongo {

#define TEST_CLASS MongoPluginManagerTests

	namespace {
		template<typename TAction>
		void RunPluginManagerTest(const model::BlockChainConfiguration& config, TAction action) {
			// Arrange:
			// - windows requires the caller to explicitly create a mongocxx instance before certain operations
			//   like creating a mongocxx::pool (via MongoStorageContext)
			mongocxx::instance::current();
			MongoStorageContext mongoContext(test::DefaultDbUri(), "", nullptr);

			MongoPluginManager manager(mongoContext, config);

			// Act + Assert:
			action(manager, mongoContext);
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
		RunPluginManagerTest(config, [](const auto& manager, const auto& mongoContext) {
			// Assert: compare BlockPruneInterval as a sentinel value because the manager copies the config
			EXPECT_EQ(&mongoContext, &manager.mongoContext());
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
}}
