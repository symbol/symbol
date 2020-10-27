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

#include "BlockChainConfiguration.h"
#include "Address.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

DEFINE_ADDRESS_CONFIGURATION_VALUE_SUPPORT

namespace catapult { namespace model {

	// region BlockChainConfiguration

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
		LOAD_NETWORK_PROPERTY(NodeEqualityStrategy);
		LOAD_NETWORK_PROPERTY(NemesisSignerPublicKey);
		LOAD_NETWORK_PROPERTY(GenerationHashSeed);
		LOAD_NETWORK_PROPERTY(EpochAdjustment);

#undef LOAD_NETWORK_PROPERTY

#define LOAD_CHAIN_PROPERTY(NAME) utils::LoadIniProperty(bag, "chain", #NAME, config.NAME)

		LOAD_CHAIN_PROPERTY(EnableVerifiableState);
		LOAD_CHAIN_PROPERTY(EnableVerifiableReceipts);

		LOAD_CHAIN_PROPERTY(CurrencyMosaicId);
		LOAD_CHAIN_PROPERTY(HarvestingMosaicId);

		LOAD_CHAIN_PROPERTY(BlockGenerationTargetTime);
		LOAD_CHAIN_PROPERTY(BlockTimeSmoothingFactor);

		LOAD_CHAIN_PROPERTY(ImportanceGrouping);
		LOAD_CHAIN_PROPERTY(ImportanceActivityPercentage);
		LOAD_CHAIN_PROPERTY(MaxRollbackBlocks);
		LOAD_CHAIN_PROPERTY(MaxDifficultyBlocks);
		LOAD_CHAIN_PROPERTY(DefaultDynamicFeeMultiplier);

		LOAD_CHAIN_PROPERTY(MaxTransactionLifetime);
		LOAD_CHAIN_PROPERTY(MaxBlockFutureTime);

		LOAD_CHAIN_PROPERTY(InitialCurrencyAtomicUnits);
		LOAD_CHAIN_PROPERTY(MaxMosaicAtomicUnits);

		LOAD_CHAIN_PROPERTY(TotalChainImportance);
		LOAD_CHAIN_PROPERTY(MinHarvesterBalance);
		LOAD_CHAIN_PROPERTY(MaxHarvesterBalance);
		LOAD_CHAIN_PROPERTY(MinVoterBalance);

		LOAD_CHAIN_PROPERTY(VotingSetGrouping);
		LOAD_CHAIN_PROPERTY(MaxVotingKeysPerAccount);
		LOAD_CHAIN_PROPERTY(MinVotingKeyLifetime);
		LOAD_CHAIN_PROPERTY(MaxVotingKeyLifetime);

		LOAD_CHAIN_PROPERTY(HarvestBeneficiaryPercentage);
		LOAD_CHAIN_PROPERTY(HarvestNetworkPercentage);
		LOAD_CHAIN_PROPERTY(HarvestNetworkFeeSinkAddress);

		LOAD_CHAIN_PROPERTY(MaxTransactionsPerBlock);

#undef LOAD_CHAIN_PROPERTY

#define LOAD_FORK_HEIGHT_PROPERTY(NAME) utils::LoadIniProperty(bag, "fork_heights", #NAME, config.ForkHeights.NAME)

		LOAD_FORK_HEIGHT_PROPERTY(VotingKeyLinkV2);

#undef LOAD_FORK_HEIGHT_PROPERTY

		size_t numPluginProperties = 0;
		for (const auto& section : bag.sections()) {
			if ("network" == section || "chain" == section || "fork_heights" == section)
				continue;

			std::string prefix("plugin:");
			if (section.size() <= prefix.size() || 0 != section.find(prefix))
				CATAPULT_THROW_INVALID_ARGUMENT_1("configuration bag contains unexpected section", section);

			auto pluginName = section.substr(prefix.size());
			CheckPluginName(pluginName);
			auto iter = config.Plugins.emplace(pluginName, utils::ExtractSectionAsBag(bag, section.c_str())).first;
			numPluginProperties += iter->second.size();
		}

		utils::VerifyBagSizeExact(bag, 5 + 27 + 1 + numPluginProperties);
		return config;
	}

	// endregion

	// region calculated properties

	UnresolvedMosaicId GetUnresolvedCurrencyMosaicId(const BlockChainConfiguration& config) {
		return UnresolvedMosaicId(config.CurrencyMosaicId.unwrap());
	}

	namespace {
		utils::TimeSpan CalculateFullRollbackDuration(const BlockChainConfiguration& config) {
			return utils::TimeSpan::FromMilliseconds(config.BlockGenerationTargetTime.millis() * config.MaxRollbackBlocks);
		}

		utils::TimeSpan CalculateRollbackVariabilityBufferDuration(const BlockChainConfiguration& config) {
			// use the greater of 25% of the rollback time or one hour as a buffer against block time variability
			return utils::TimeSpan::FromHours(4).millis() > CalculateFullRollbackDuration(config).millis()
					? utils::TimeSpan::FromHours(1)
					: utils::TimeSpan::FromMilliseconds(CalculateFullRollbackDuration(config).millis() / 4);
		}
	}

	utils::TimeSpan CalculateTransactionCacheDuration(const BlockChainConfiguration& config) {
		if (0 == config.MaxRollbackBlocks)
			return config.MaxTransactionLifetime;

		return utils::TimeSpan::FromMilliseconds(
				CalculateFullRollbackDuration(config).millis()
				+ CalculateRollbackVariabilityBufferDuration(config).millis());
	}

	// endregion
}}
