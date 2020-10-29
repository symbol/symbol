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

#include "NemesisCompatibleConfiguration.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/TestNetworkConstants.h"

namespace catapult { namespace test {

	namespace {
		model::BlockChainConfiguration CreateBlockChainConfiguration() {
			auto config = CreatePrototypicalBlockChainConfiguration();
			AddNemesisPluginExtensions(config);
			return config;
		}
	}

	void AddNemesisPluginExtensions(model::BlockChainConfiguration& config) {
		config.Plugins.emplace("catapult.plugins.transfer", utils::ConfigurationBag({{ "", { { "maxMessageSize", "0" } } }}));
		config.Plugins.emplace("catapult.plugins.mosaic", utils::ConfigurationBag({{
			"",
			{
				{ "maxMosaicsPerAccount", "123" },
				{ "maxMosaicDuration", "456d" },
				{ "maxMosaicDivisibility", "6" },

				{ "mosaicRentalFeeSinkAddress", Mosaic_Rental_Fee_Sink_Address },
				{ "mosaicRentalFee", "500" }
			}
		}}));
		config.Plugins.emplace("catapult.plugins.namespace", utils::ConfigurationBag({{
			"",
			{
				{ "maxNameSize", "64" },
				{ "maxChildNamespaces", "100" },
				{ "maxNamespaceDepth", "3" },

				{ "minNamespaceDuration", "1m" },
				{ "maxNamespaceDuration", "365d" },
				{ "namespaceGracePeriodDuration", "1h" },
				{ "reservedRootNamespaceNames", "cat" },

				{ "namespaceRentalFeeSinkAddress", Namespace_Rental_Fee_Sink_Address },
				{ "rootNamespaceRentalFeePerBlock", "10" },
				{ "childNamespaceRentalFee", "10000" }
			}
		}}));
	}

	namespace {
		void AddPluginExtensions(config::ExtensionsConfiguration& config, const std::unordered_set<std::string>& extensionNames) {
			for (const auto& extensionName : extensionNames)
				config.Names.emplace_back("extension." + extensionName);
		}

		void AddCommonPluginExtensions(config::ExtensionsConfiguration& config) {
			// finalization is needed because int tests are run with 0 == MaxRollbackBlocks
			AddPluginExtensions(config, { "diagnostics", "finalization", "packetserver", "sync", "transactionsink" });
		}
	}

	void AddApiPluginExtensions(config::ExtensionsConfiguration& config) {
		AddCommonPluginExtensions(config);
	}

	void AddPeerPluginExtensions(config::ExtensionsConfiguration& config) {
		AddCommonPluginExtensions(config);
		AddPluginExtensions(config, { "harvesting", "syncsource" });
	}

	void AddSimplePartnerPluginExtensions(config::ExtensionsConfiguration& config) {
		AddPluginExtensions(config, { "finalization", "packetserver", "sync", "syncsource" });
	}

	void AddRecoveryPluginExtensions(config::ExtensionsConfiguration& config) {
		AddPluginExtensions(config, {});
	}

	config::CatapultConfiguration CreateCatapultConfigurationWithNemesisPluginExtensions(const std::string& dataDirectory) {
		return CreatePrototypicalCatapultConfiguration(CreateBlockChainConfiguration(), dataDirectory);
	}
}}
