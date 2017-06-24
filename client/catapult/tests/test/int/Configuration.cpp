#include "Configuration.h"
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

			{ "maxMosaicDuration", "456d" },

			{ "isMosaicLevyUpdateAllowed", "true" },
			{ "maxMosaicDivisibility", "6" },
			{ "maxMosaicDivisibleUnits", "9'000'000'000'000'000" },

			{ "mosaicRentalFeeSinkPublicKey", Mosaic_Rental_Fee_Sink_Public_Key },
			{ "mosaicRentalFee", "500" }
		} } }));
	}

	config::LocalNodeConfiguration LoadLocalNodeConfigurationWithNemesisPluginExtensions(const std::string& dataDirectory) {
		return LoadLocalNodeConfiguration(CreateBlockChainConfiguration(), local_node_flags::None, dataDirectory);
	}
}}
