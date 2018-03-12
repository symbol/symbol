#include "catapult/plugins/PluginLoader.h"
#include "catapult/plugins/PluginExceptions.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/utils/ExceptionLogging.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace plugins {

#define TEST_CLASS PluginLoaderTests

	namespace {
		constexpr auto Known_Plugin_Name = "catapult.plugins.transfer";

		void AssertCanLoadPlugins(
				const std::string& directory,
				const model::BlockChainConfiguration& config,
				bool isDynamicModule,
				const std::vector<std::string>& pluginNames) {
			// Arrange: ensure module is destroyed after manager
			for (const auto& name : pluginNames) {
				PluginModules modules;
				PluginManager manager(config);
				CATAPULT_LOG(debug) << "loading plugin with name: " << name;

				// Act:
				LoadPluginByName(manager, modules, directory, name);

				// Assert: all known plugins have at least one observer or (stateless) validator
				EXPECT_FALSE(manager.createObserver()->names().empty() && manager.createStatelessValidator()->names().empty());

				// - check the module
				ASSERT_EQ(1u, modules.size());
				EXPECT_EQ(isDynamicModule, modules.back().isLoaded());
			}
		}

		void AssertCanLoadKnownStaticallyLinkedPlugins(const std::string& directory) {
			// Assert:
			AssertCanLoadPlugins(directory, model::BlockChainConfiguration::Uninitialized(), false, { "catapult.coresystem" });
		}

		void AssertCanLoadKnownDynamicallyLinkedPlugins(const std::string& directory) {
			// Arrange:
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Plugins.emplace(Known_Plugin_Name, utils::ConfigurationBag({{ "", { { "maxMessageSize", "0" } } }}));

			// Assert:
			AssertCanLoadPlugins(directory, config, true, { Known_Plugin_Name });
		}

		void AssertCannotLoadUnknownPlugin(const std::string& directory) {
			// Arrange:
			PluginModules modules;
			PluginManager manager(model::BlockChainConfiguration::Uninitialized());

			// Act + Assert:
			EXPECT_THROW(LoadPluginByName(manager, modules, directory, "catapult.plugins.awesome"), catapult_invalid_argument);
		}
	}

	TEST(TEST_CLASS, CanLoadKnownStaticallyLinkedPlugins_ExplicitDirectory) {
		// Assert:
		AssertCanLoadKnownStaticallyLinkedPlugins(test::GetExplicitPluginsDirectory());
	}

	TEST(TEST_CLASS, CanLoadKnownStaticallyLinkedPlugins_ImplicitDirectory) {
		// Assert:
		AssertCanLoadKnownStaticallyLinkedPlugins("");
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
			// - prepare insufficient configuration
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Plugins.emplace(Known_Plugin_Name, utils::ConfigurationBag({{ "", { { "maxMessageSizeX", "0" } } }}));

			// - create the manager
			PluginModules modules;
			PluginManager manager(config);

			// Act:
			LoadPluginByName(manager, modules, "", Known_Plugin_Name);
		} catch (const plugin_load_error&) {
			CATAPULT_LOG(debug) << UNHANDLED_EXCEPTION_MESSAGE("while running test");
			isExceptionHandled = true;
		}

		// Assert:
		EXPECT_TRUE(isExceptionHandled);
	}
}}
