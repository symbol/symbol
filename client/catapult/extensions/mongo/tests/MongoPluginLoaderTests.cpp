#include "mongo/src/MongoPluginLoader.h"
#include "mongo/src/MongoPluginManager.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/plugins/PluginExceptions.h"
#include "catapult/utils/ExceptionLogging.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <mongocxx/instance.hpp>

namespace catapult { namespace mongo {

#define TEST_CLASS MongoPluginLoaderTests

	namespace {
		constexpr auto Known_Plugin_Name = "catapult.mongo.plugins.transfer";

		template<typename TAction>
		void RunPluginManagerTest(TAction action) {
			// Arrange:
			// - windows requires the caller to explicitly create a mongocxx instance before certain operations
			//   like creating a mongocxx::pool (via MongoStorageContext)
			mongocxx::instance::current();
			MongoStorageContext mongoContext(test::DefaultDbUri(), "", nullptr);

			MongoPluginManager manager(mongoContext, model::BlockChainConfiguration::Uninitialized());

			// Act + Assert:
			action(manager);
		}

		void AssertCanLoadKnownDynamicallyLinkedPlugins(const std::string& directory) {
			// Arrange: ensure module is destroyed after manager
			PluginModules modules;
			RunPluginManagerTest([&directory, &modules](auto& manager) {
				// Act:
				LoadPluginByName(manager, modules, directory, Known_Plugin_Name);

				// Assert:
				EXPECT_EQ(1u, manager.transactionRegistry().size());

				// - check the module
				ASSERT_EQ(1u, modules.size());
				EXPECT_TRUE(modules.back().isLoaded());
			});
		}

		void AssertCannotLoadUnknownPlugin(const std::string& directory) {
			// Arrange:
			PluginModules modules;
			RunPluginManagerTest([&directory, &modules](auto& manager) {
				// Act + Assert:
				EXPECT_THROW(LoadPluginByName(manager, modules, directory, "catapult.mongo.plugins.awesome"), catapult_invalid_argument);
			});
		}
	}

	TEST(TEST_CLASS, CanLoadKnownDynamicallyLinkedPlugins_ExplicitDirectory) {
		// Assert:
		AssertCanLoadKnownDynamicallyLinkedPlugins(test::GetExplicitPluginsDirectory());
	}

	TEST(TEST_CLASS, CanLoadKnownDynamicallyLinkedPlugins_ImplicitDirectory) {
		// Assert:
		AssertCanLoadKnownDynamicallyLinkedPlugins("");
	}

	TEST(TEST_CLASS, CannotLoadUnknownPlugin_ExplicitDirectory) {
		// Assert:
		AssertCannotLoadUnknownPlugin(test::GetExplicitPluginsDirectory());
	}

	TEST(TEST_CLASS, CannotLoadUnknownPlugin_ImplicitDirectory) {
		// Assert:
		AssertCannotLoadUnknownPlugin("");
	}

	TEST(TEST_CLASS, PluginRegistrationExceptionIsForwarded) {
		// Arrange:
		bool isExceptionHandled = false;
		try {
			// - create the manager
			PluginModules modules;
			RunPluginManagerTest([&modules](auto& manager) {
				// Act: force an exception by loading the same module twice
				LoadPluginByName(manager, modules, "", Known_Plugin_Name);
				LoadPluginByName(manager, modules, "", Known_Plugin_Name);
			});
		} catch (const plugins::plugin_load_error&) {
			CATAPULT_LOG(debug) << UNHANDLED_EXCEPTION_MESSAGE("while running test");
			isExceptionHandled = true;
		}

		// Assert:
		EXPECT_TRUE(isExceptionHandled);
	}
}}
