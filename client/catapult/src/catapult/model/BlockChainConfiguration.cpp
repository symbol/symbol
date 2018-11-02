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

#include "BlockChainConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace model {

	namespace {
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

		LOAD_CHAIN_PROPERTY(ShouldEnableVerifiableState);

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

			std::string prefix("plugin:");
			if (section.size() <= prefix.size() || 0 != section.find(prefix))
				CATAPULT_THROW_INVALID_ARGUMENT_1("configuration bag contains unexpected section", section);

			auto pluginName = section.substr(prefix.size());
			CheckPluginName(pluginName);
			auto iter = config.Plugins.emplace(pluginName, utils::ExtractSectionAsBag(bag, section.c_str())).first;
			numPluginProperties += iter->second.size();
		}

		utils::VerifyBagSizeLte(bag, 15 + numPluginProperties);
		return config;
	}
}}
