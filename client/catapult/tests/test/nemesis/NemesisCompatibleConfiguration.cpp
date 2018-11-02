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
#include "tests/test/nodeps/MijinConstants.h"

namespace catapult { namespace test {

	namespace {
		model::BlockChainConfiguration CreateBlockChainConfiguration() {
			auto config = CreateLocalNodeBlockChainConfiguration();
			AddNemesisPluginExtensions(config);
			return config;
		}
	}

	void AddNemesisPluginExtensions(model::BlockChainConfiguration& config) {
		config.Plugins.emplace("catapult.plugins.transfer", utils::ConfigurationBag({{ "", { { "maxMessageSize", "0" } } }}));
		config.Plugins.emplace("catapult.plugins.namespace", utils::ConfigurationBag({ { "", {
			{ "maxNameSize", "64" },

			{ "maxNamespaceDuration", "365d" },
			{ "namespaceGracePeriodDuration", "1h" },
			{ "reservedRootNamespaceNames", "xem" },

			{ "namespaceRentalFeeSinkPublicKey", Namespace_Rental_Fee_Sink_Public_Key },
			{ "rootNamespaceRentalFeePerBlock", "10" },
			{ "childNamespaceRentalFee", "10000" },

			{ "maxChildNamespaces", "100" },
			{ "maxMosaicsPerAccount", "123" },

			{ "maxMosaicDuration", "456d" },

			{ "isMosaicLevyUpdateAllowed", "true" },
			{ "maxMosaicDivisibility", "6" },
			{ "maxMosaicDivisibleUnits", "9'000'000'000'000'000" },

			{ "mosaicRentalFeeSinkPublicKey", Mosaic_Rental_Fee_Sink_Public_Key },
			{ "mosaicRentalFee", "500" }
		} } }));
	}

	namespace {
		void AddPluginExtensions(config::NodeConfiguration& config, const std::unordered_set<std::string>& extensionNames) {
			for (const auto& extensionName : extensionNames)
				config.Extensions.emplace_back("extension." + extensionName);
		}

		void AddCommonPluginExtensions(config::NodeConfiguration& config) {
			AddPluginExtensions(config, { "diagnostics", "filechain", "networkheight", "packetserver", "sync", "transactionsink" });
		}
	}

	void AddApiPluginExtensions(config::NodeConfiguration& config) {
		AddCommonPluginExtensions(config);
	}

	void AddPeerPluginExtensions(config::NodeConfiguration& config) {
		AddCommonPluginExtensions(config);
		AddPluginExtensions(config, { "eventsource", "harvesting", "syncsource" });
	}

	void AddSimplePartnerPluginExtensions(config::NodeConfiguration& config) {
		AddPluginExtensions(config, { "filechain", "packetserver", "sync", "syncsource" });
	}

	void EnableStateVerification(config::LocalNodeConfiguration& config) {
		const_cast<config::NodeConfiguration&>(config.Node).ShouldUseCacheDatabaseStorage = true;
		const_cast<model::BlockChainConfiguration&>(config.BlockChain).ShouldEnableVerifiableState = true;
	}

	config::LocalNodeConfiguration CreateLocalNodeConfigurationWithNemesisPluginExtensions(const std::string& dataDirectory) {
		return CreateLocalNodeConfiguration(CreateBlockChainConfiguration(), dataDirectory);
	}
}}
