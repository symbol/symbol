#include "catapult/config/ValidateConfiguration.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		// the key is invalid because it contains a non hex char ('G')
		const char* Invalid_Private_Key = "3485d98efd7eb07adafcfd1a157d89de2G96a95e780813c0258af3f5f84ed8cb";
		const char* Valid_Private_Key = "3485d98efd7eb07adafcfd1a157d89de2796a95e780813c0258af3f5f84ed8cb";

		auto CreateValidNodeConfiguration() {
			return NodeConfiguration::Uninitialized();
		}

		auto CreateValidUserConfiguration() {
			auto userConfig = UserConfiguration::Uninitialized();
			userConfig.BootKey = Valid_Private_Key;
			userConfig.HarvestKey = Valid_Private_Key;
			userConfig.IsAutoHarvestingEnabled = false;
			return userConfig;
		}

		auto CreateImportanceGroupingConfiguration(
				uint32_t importanceGrouping,
				uint32_t maxRollbackBlocks) {
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			blockChainConfig.ImportanceGrouping = importanceGrouping;
			blockChainConfig.MaxRollbackBlocks = maxRollbackBlocks;
			return blockChainConfig;
		}

		auto CreateAndValidateLocalNodeConfiguration(
				UserConfiguration&& userConfig,
				NodeConfiguration&& nodeConfig = CreateValidNodeConfiguration(),
				size_t numPeers = 1) {
			// Arrange:
			std::vector<ionet::Node> peers;
			for (auto i = 0u; i < numPeers; ++i)
				peers.push_back(test::CreateLocalHostNode(test::GenerateRandomData<Key_Size>()));

			// Act:
			auto config = LocalNodeConfiguration(
					CreateImportanceGroupingConfiguration(1, 0),
					std::move(nodeConfig),
					LoggingConfiguration::Uninitialized(),
					std::move(userConfig),
					std::move(peers));
			ValidateConfiguration(config);
		}

		auto CreateAndValidateLocalNodeConfiguration(model::BlockChainConfiguration&& blockChainConfig) {
			// Act:
			auto config = LocalNodeConfiguration(
					std::move(blockChainConfig),
					CreateValidNodeConfiguration(),
					LoggingConfiguration::Uninitialized(),
					CreateValidUserConfiguration(),
					{ test::CreateLocalHostNode(test::GenerateRandomData<Key_Size>()) });
			ValidateConfiguration(config);
		}
	}

	// region boot key validation

	namespace {
		void AssertInvalidBootKey(const std::string& bootKey) {
			// Arrange:
			auto userConfig = UserConfiguration::Uninitialized();
			userConfig.BootKey = bootKey;
			userConfig.IsAutoHarvestingEnabled = false;

			// Act:
			EXPECT_THROW(CreateAndValidateLocalNodeConfiguration(std::move(userConfig)), utils::property_malformed_error);
		}
	}

	TEST(ValidateConfigurationTests, ValidationFailsIfBootKeyIsInvalid) {
		// Assert:
		AssertInvalidBootKey(Invalid_Private_Key);
		AssertInvalidBootKey("");
	}

	// endregion

	// region harvest key validation

	namespace {
		void AssertInvalidHarvestKey(const std::string& harvestKey, bool isAutoHarvestingEnabled) {
			// Arrange:
			auto userConfig = UserConfiguration::Uninitialized();
			userConfig.BootKey = Valid_Private_Key;
			userConfig.HarvestKey = harvestKey;
			userConfig.IsAutoHarvestingEnabled = isAutoHarvestingEnabled;

			// Act:
			EXPECT_THROW(CreateAndValidateLocalNodeConfiguration(std::move(userConfig)), utils::property_malformed_error);
		}
	}

	TEST(ValidateConfigurationTests, ValidationFailsIfHarvestKeyIsInvalid) {
		// Assert:
		AssertInvalidHarvestKey(Invalid_Private_Key, true);
		AssertInvalidHarvestKey(Invalid_Private_Key, false);
	}

	TEST(ValidateConfigurationTests, ValidationFailsIfHarvestKeyIsUnspecifiedAndAutoHarvestingIsEnabled) {
		// Assert:
		AssertInvalidHarvestKey("", true);
	}

	TEST(ValidateConfigurationTests, ValidationSucceedsIfHarvestKeyIsUnspecifiedAndAutoHarvestingIsDisabled) {
		// Arrange:
		auto userConfig = CreateValidUserConfiguration();
		userConfig.BootKey = Valid_Private_Key;
		userConfig.HarvestKey = "";
		userConfig.IsAutoHarvestingEnabled = false;

		// Act + Assert: no exception
		CreateAndValidateLocalNodeConfiguration(std::move(userConfig));
	}

	// endregion

	// region peer validation

	TEST(ValidateConfigurationTests, ValidationSucceedsIfNodeHasNoPeers) {
		// Arrange:
		auto userConfig = CreateValidUserConfiguration();
		auto nodeConfig = CreateValidNodeConfiguration();

		// Act + Assert: no exception
		CreateAndValidateLocalNodeConfiguration(std::move(userConfig), std::move(nodeConfig), 0);
	}

	// endregion

	// region importance grouping validation

	TEST(ValidateConfigurationTests, ImportanceGroupingIsValidatedAgainstMaxRollbackBlocks) {
		// Arrange:
		auto assertNoThrow = [](auto importanceGrouping, auto maxRollbackBlocks) {
			auto blockChainConfig = CreateImportanceGroupingConfiguration(importanceGrouping, maxRollbackBlocks);
			EXPECT_NO_THROW(CreateAndValidateLocalNodeConfiguration(std::move(blockChainConfig)))
					<< "IG " << importanceGrouping << ", MRB " << maxRollbackBlocks;
		};

		auto assertThrow = [](auto importanceGrouping, auto maxRollbackBlocks) {
			auto blockChainConfig = CreateImportanceGroupingConfiguration(importanceGrouping, maxRollbackBlocks);
			EXPECT_THROW(CreateAndValidateLocalNodeConfiguration(std::move(blockChainConfig)), utils::property_malformed_error)
					<< "IG " << importanceGrouping << ", MRB " << maxRollbackBlocks;
		};

		// Act + Assert:
		// - no exceptions
		assertNoThrow(181u, 360u); // 2 * IG > MRB
		assertNoThrow(400u, 360u); // IG > MRB

		// - exceptions
		assertThrow(0u, 360u); // 0 IG
		assertThrow(180u, 360u); // 2 * IG == MRB
		assertThrow(179u, 360u); // 2 * IG < MRB
	}

	// endregion
}}
