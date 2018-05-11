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

#include "catapult/config/ValidateConfiguration.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

#define TEST_CLASS ValidateConfigurationTests

	namespace {
		// the key is invalid because it contains a non hex char ('G')
		const char* Invalid_Private_Key = "3485D98EFD7EB07ABAFCFD1A157D89DE2G96A95E780813C0258AF3F5F84ED8CB";
		const char* Valid_Private_Key = "3485D98EFD7EB07ABAFCFD1A157D89DE2796A95E780813C0258AF3F5F84ED8CB";

		auto CreateValidNodeConfiguration() {
			return NodeConfiguration::Uninitialized();
		}

		auto CreateValidUserConfiguration() {
			auto userConfig = UserConfiguration::Uninitialized();
			userConfig.BootKey = Valid_Private_Key;
			return userConfig;
		}

		auto CreateImportanceGroupingConfiguration(uint32_t importanceGrouping, uint32_t maxRollbackBlocks) {
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			blockChainConfig.ImportanceGrouping = importanceGrouping;
			blockChainConfig.MaxRollbackBlocks = maxRollbackBlocks;
			return blockChainConfig;
		}

		auto CreateAndValidateLocalNodeConfiguration(
				UserConfiguration&& userConfig,
				NodeConfiguration&& nodeConfig = CreateValidNodeConfiguration()) {
			// Act:
			auto config = LocalNodeConfiguration(
					CreateImportanceGroupingConfiguration(1, 0),
					std::move(nodeConfig),
					LoggingConfiguration::Uninitialized(),
					std::move(userConfig));
			ValidateConfiguration(config);
		}

		auto CreateAndValidateLocalNodeConfiguration(model::BlockChainConfiguration&& blockChainConfig) {
			// Act:
			auto config = LocalNodeConfiguration(
					std::move(blockChainConfig),
					CreateValidNodeConfiguration(),
					LoggingConfiguration::Uninitialized(),
					CreateValidUserConfiguration());
			ValidateConfiguration(config);
		}
	}

	// region boot key validation

	namespace {
		void AssertInvalidBootKey(const std::string& bootKey) {
			// Arrange:
			auto userConfig = UserConfiguration::Uninitialized();
			userConfig.BootKey = bootKey;

			// Act + Assert:
			EXPECT_THROW(CreateAndValidateLocalNodeConfiguration(std::move(userConfig)), utils::property_malformed_error);
		}
	}

	TEST(TEST_CLASS, ValidationFailsIfBootKeyIsInvalid) {
		// Assert:
		AssertInvalidBootKey(Invalid_Private_Key);
		AssertInvalidBootKey("");
	}

	// endregion

	// region importance grouping validation

	TEST(TEST_CLASS, ImportanceGroupingIsValidatedAgainstMaxRollbackBlocks) {
		// Arrange:
		auto assertNoThrow = [](uint32_t importanceGrouping, uint32_t maxRollbackBlocks) {
			auto blockChainConfig = CreateImportanceGroupingConfiguration(importanceGrouping, maxRollbackBlocks);
			EXPECT_NO_THROW(CreateAndValidateLocalNodeConfiguration(std::move(blockChainConfig)))
					<< "IG " << importanceGrouping << ", MRB " << maxRollbackBlocks;
		};

		auto assertThrow = [](uint32_t importanceGrouping, uint32_t maxRollbackBlocks) {
			auto blockChainConfig = CreateImportanceGroupingConfiguration(importanceGrouping, maxRollbackBlocks);
			EXPECT_THROW(CreateAndValidateLocalNodeConfiguration(std::move(blockChainConfig)), utils::property_malformed_error)
					<< "IG " << importanceGrouping << ", MRB " << maxRollbackBlocks;
		};

		// Act + Assert:
		// - no exceptions
		assertNoThrow(181, 360); // 2 * IG > MRB
		assertNoThrow(400, 360); // IG > MRB

		// - exceptions
		assertThrow(0, 360); // 0 IG
		assertThrow(180, 360); // 2 * IG == MRB
		assertThrow(179, 360); // 2 * IG < MRB
	}

	// endregion
}}
