#include "BlockChainConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace model {

	namespace {
		const RawString Plugin_Section_Prefix("plugin:");

		void CheckPluginName(const std::string& pluginName) {
			if (std::any_of(pluginName.cbegin(), pluginName.cend(), [](auto ch) { return (ch < 'a' || ch > 'z') && '.' != ch; }))
				CATAPULT_THROW_INVALID_ARGUMENT_1("plugin name contains unexpected character", pluginName);
		}
	}

	BlockChainConfiguration BlockChainConfiguration::Uninitialized() {
		return BlockChainConfiguration();
	}

	BlockChainConfiguration BlockChainConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		BlockChainConfiguration config;

#define LOAD_NETWORK_PROPERTY(NAME) utils::LoadIniProperty(bag, "network", #NAME, config.Network.NAME)

		LOAD_NETWORK_PROPERTY(Identifier);
		LOAD_NETWORK_PROPERTY(PublicKey);
		LOAD_NETWORK_PROPERTY(GenerationHash);

#undef LOAD_NETWORK_PROPERTY

#define LOAD_CHAIN_PROPERTY(NAME) utils::LoadIniProperty(bag, "chain", #NAME, config.NAME)

		LOAD_CHAIN_PROPERTY(BlockGenerationTargetTime);
		LOAD_CHAIN_PROPERTY(BlockTimeSmoothingFactor);

		LOAD_CHAIN_PROPERTY(ImportanceGrouping);
		LOAD_CHAIN_PROPERTY(MaxRollbackBlocks);
		LOAD_CHAIN_PROPERTY(MaxDifficultyBlocks);

		LOAD_CHAIN_PROPERTY(MaxTransactionLifetime);
		LOAD_CHAIN_PROPERTY(MaxBlockFutureTime);

		LOAD_CHAIN_PROPERTY(TotalChainBalance);
		LOAD_CHAIN_PROPERTY(MinHarvesterBalance);

		LOAD_CHAIN_PROPERTY(BlockPruneInterval);
		LOAD_CHAIN_PROPERTY(MaxTransactionsPerBlock);

#undef LOAD_CHAIN_PROPERTY

		size_t numPluginProperties = 0;
		for (const auto& section : bag.sections()) {
			if ("network" == section || "chain" == section)
				continue;

			const auto& prefix = Plugin_Section_Prefix;
			if (section.size() <= prefix.Size || 0 != memcmp(section.data(), prefix.pData, prefix.Size))
				CATAPULT_THROW_INVALID_ARGUMENT_1("configuration bag contains unexpected section", section);

			auto pluginName = section.substr(prefix.Size);
			CheckPluginName(pluginName);
			auto iter = config.Plugins.emplace(pluginName, utils::ExtractSectionAsBag(bag, section.c_str())).first;
			numPluginProperties += iter->second.size();
		}

		utils::VerifyBagSizeLte(bag, 14 + numPluginProperties);
		return config;
	}
}}
