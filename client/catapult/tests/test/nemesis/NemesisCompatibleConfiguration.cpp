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
			{ "namespaceGracePeriodDuration", "30d" },
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
				config.Extensions.emplace("extension." + extensionName);
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

	config::LocalNodeConfiguration LoadLocalNodeConfigurationWithNemesisPluginExtensions(const std::string& dataDirectory) {
		return LoadLocalNodeConfiguration(CreateBlockChainConfiguration(), dataDirectory);
	}
}}
