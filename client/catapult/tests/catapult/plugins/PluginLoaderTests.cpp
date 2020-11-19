/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/plugins/PluginLoader.h"
#include "catapult/plugins/PluginExceptions.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/utils/ExceptionLogging.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/plugins/PluginManagerFactory.h"
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
				auto manager = test::CreatePluginManager(config);
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
			auto manager = test::CreatePluginManager();

			// Act + Assert:
			EXPECT_THROW(LoadPluginByName(manager, modules, directory, "catapult.plugins.awesome"), catapult_invalid_argument);
		}
	}

	TEST(TEST_CLASS, CanLoadKnownDynamicallyLinkedPlugins_ExplicitDirectory) {
		AssertCanLoadKnownDynamicallyLinkedPlugins(test::GetExplicitPluginsDirectory());
	}

	TEST(TEST_CLASS, CanLoadKnownDynamicallyLinkedPlugins_ImplicitDirectory) {
		AssertCanLoadKnownDynamicallyLinkedPlugins("");
	}

	TEST(TEST_CLASS, CannotLoadUnknownPlugin_ExplicitDirectory) {
		AssertCannotLoadUnknownPlugin(test::GetExplicitPluginsDirectory());
	}

	TEST(TEST_CLASS, CannotLoadUnknownPlugin_ImplicitDirectory) {
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
			auto manager = test::CreatePluginManager(config);

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
