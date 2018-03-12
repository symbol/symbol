#include "catapult/extensions/LocalNodeBootstrapper.h"
#include "catapult/plugins/PluginExceptions.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS LocalNodeBootstrapperTests

	// region basic

	TEST(TEST_CLASS, CanCreateBootstrapper) {
		// Arrange:
		auto config = test::CreateUninitializedLocalNodeConfiguration();
		const_cast<uint32_t&>(config.BlockChain.BlockPruneInterval) = 15;

		// Act:
		LocalNodeBootstrapper bootstrapper(config, "resources path", "bootstrapper");

		// Assert: compare BlockPruneInterval as a sentinel value because the bootstrapper copies the config
		EXPECT_EQ(15u, bootstrapper.config().BlockChain.BlockPruneInterval);
		EXPECT_EQ(15u, bootstrapper.pluginManager().config().BlockPruneInterval);

		// - resources path should be correct
		EXPECT_EQ("resources path", bootstrapper.resourcesPath());

		// - nodes should be empty
		EXPECT_TRUE(bootstrapper.staticNodes().empty());

		// - pool should have default number of threads
		EXPECT_EQ(std::thread::hardware_concurrency(), bootstrapper.pool().numWorkerThreads());

		// - other managers should not throw
		bootstrapper.extensionManager();
		bootstrapper.subscriptionManager();
	}

	// endregion

	// region loadExtensions

	namespace {
		template<typename TAction>
		void RunExtensionsTest(const std::string& directory, const std::string& name, TAction action) {
			// Arrange:
			auto config = test::CreateUninitializedLocalNodeConfiguration();
			const_cast<config::UserConfiguration&>(config.User).PluginsDirectory = directory;
			const_cast<config::NodeConfiguration&>(config.Node).Extensions = { name };
			LocalNodeBootstrapper bootstrapper(config, "resources path", "bootstrapper");

			// Act + Assert:
			action(bootstrapper);
		}

		void AssertCanLoadKnownDynamicallyLinkedPlugins(const std::string& directory) {
			// Arrange:
			RunExtensionsTest(directory, "extension.hashcache", [](auto& bootstrapper) {
				// Sanity:
				const auto& systemPluginNames = bootstrapper.extensionManager().systemPluginNames();
				EXPECT_EQ(2u, systemPluginNames.size());

				// Act:
				bootstrapper.loadExtensions();

				// Assert: hash cache plugin registers a single system plugin
				EXPECT_EQ(3u, systemPluginNames.size());
			});
		}

		void AssertCannotLoadUnknownPlugin(const std::string& directory) {
			// Arrange:
			RunExtensionsTest(directory, "extension.awesome", [](auto& bootstrapper) {
				// Act + Assert:
				EXPECT_THROW(bootstrapper.loadExtensions(), catapult_invalid_argument);
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
			// - create the bootstrapper
			RunExtensionsTest("", "extension.harvesting", [](auto& bootstrapper) {
				// Act: force an exception by loading harvesting extension with improper resources path
				bootstrapper.loadExtensions();
			});
		} catch (const plugins::plugin_load_error&) {
			CATAPULT_LOG(debug) << UNHANDLED_EXCEPTION_MESSAGE("while running test");
			isExceptionHandled = true;
		}

		// Assert:
		EXPECT_TRUE(isExceptionHandled);
	}

	// endregion

	// region staticNodes + AddStaticNodesFromPath

	TEST(TEST_CLASS, CanAddStaticNodes) {
		// Arrange:
		LocalNodeBootstrapper bootstrapper(test::CreateUninitializedLocalNodeConfiguration(), "", "bootstrapper");

		// - add five nodes
		std::vector<ionet::Node> nodes1{
			test::CreateLocalHostNode(test::GenerateRandomData<Key_Size>()),
			test::CreateLocalHostNode(test::GenerateRandomData<Key_Size>())
		};

		std::vector<ionet::Node> nodes2{
			test::CreateLocalHostNode(test::GenerateRandomData<Key_Size>()),
			test::CreateLocalHostNode(test::GenerateRandomData<Key_Size>()),
			test::CreateLocalHostNode(test::GenerateRandomData<Key_Size>())
		};

		// Act:
		bootstrapper.addStaticNodes(nodes1);
		bootstrapper.addStaticNodes(nodes2);
		const auto& bootstrapperNodes = bootstrapper.staticNodes();

		// Assert:
		ASSERT_EQ(5u, bootstrapperNodes.size());

		for (auto i = 0u; i < nodes1.size(); ++i)
			EXPECT_EQ(bootstrapperNodes[i], nodes1[i]) << "nodes1 at " << i;

		for (auto i = 0u; i < nodes2.size(); ++i)
			EXPECT_EQ(bootstrapperNodes[nodes1.size() + i], nodes2[i]) << "nodes2 at " << i;
	}

	TEST(TEST_CLASS, CanAddStaticNodesFromPath) {
		// Arrange:
		LocalNodeBootstrapper bootstrapper(test::CreateUninitializedLocalNodeConfiguration(), "", "bootstrapper");

		// Act:
		AddStaticNodesFromPath(bootstrapper, "../resources/peers-p2p.json");

		// Assert:
		EXPECT_EQ(4u, bootstrapper.staticNodes().size());
	}

	// endregion
}}
