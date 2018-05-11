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

#pragma once
#include "NetworkInfo.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/FileSize.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/utils/XemUnit.h"
#include "catapult/types.h"
#include <unordered_map>
#include <stdint.h>

namespace catapult { namespace model {

	/// Block chain configuration settings.
	struct BlockChainConfiguration {
	public:
		/// Block chain network.
		model::NetworkInfo Network;

		/// Targeted time between blocks.
		utils::TimeSpan BlockGenerationTargetTime;

		/// Smoothing factor in thousandths.
		/// If this value is non-zero, the network will be biased in favor of evenly spaced blocks.
		/// \note A higher value makes the network more biased.
		/// \note This can lower security because it will increase the influence of time relative to importance.
		uint32_t BlockTimeSmoothingFactor;

		/// Number of blocks that should be treated as a group for importance purposes.
		/// \note Importances will only be calculated at blocks that are multiples of this grouping number.
		uint64_t ImportanceGrouping;

		/// Maximum number of blocks that can be rolled back.
		uint32_t MaxRollbackBlocks;

		/// Maximum number of blocks to use in a difficulty calculation.
		uint32_t MaxDifficultyBlocks;

		/// Maximum lifetime a transaction can have before it expires.
		utils::TimeSpan MaxTransactionLifetime;

		/// Maximum future time of a block that can be accepted.
		utils::TimeSpan MaxBlockFutureTime;

		/// Total number of XEM base units available in the network.
		/// \note This quantity is specified as micro XEM in the configuration file.
		utils::XemUnit TotalChainBalance;

		/// Minimum number of micro XEM needed for an account to be eligible for harvesting.
		Amount MinHarvesterBalance;

		/// Number of blocks between cache pruning.
		uint32_t BlockPruneInterval;

		/// Maximum number of transactions per block.
		uint32_t MaxTransactionsPerBlock;

		/// Unparsed map of plugin configuration.
		std::unordered_map<std::string, utils::ConfigurationBag> Plugins;

	private:
		BlockChainConfiguration() = default;

	public:
		/// Creates an uninitialized block chain configuration.
		static BlockChainConfiguration Uninitialized();

		/// Loads a block chain configuration from \a bag.
		static BlockChainConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};

	/// Calculates the duration of a full rollback for the block chain described by \a config.
	constexpr utils::TimeSpan CalculateFullRollbackDuration(const BlockChainConfiguration& config) {
		return utils::TimeSpan::FromMilliseconds(config.BlockGenerationTargetTime.millis() * config.MaxRollbackBlocks);
	}

	/// Calculates the duration of the rollback variability buffer for the block chain described by \a config.
	constexpr utils::TimeSpan CalculateRollbackVariabilityBufferDuration(const BlockChainConfiguration& config) {
		// use the greater of 25% of the rollback time or one hour as a buffer against block time variability
		return utils::TimeSpan::FromHours(4).millis() > CalculateFullRollbackDuration(config).millis()
				? utils::TimeSpan::FromHours(1)
				: utils::TimeSpan::FromMilliseconds(CalculateFullRollbackDuration(config).millis() / 4);
	}

	/// Calculates the duration of time that expired transactions should be cached for the block chain described by \a config.
	constexpr utils::TimeSpan CalculateTransactionCacheDuration(const BlockChainConfiguration& config) {
		return utils::TimeSpan::FromMilliseconds(
				CalculateFullRollbackDuration(config).millis()
				+ CalculateRollbackVariabilityBufferDuration(config).millis());
	}

	/// Calculates the number of historical difficulties to cache in memory for the block chain described by \a config.
	constexpr uint64_t CalculateDifficultyHistorySize(const BlockChainConfiguration& config) {
		return config.MaxRollbackBlocks + config.MaxDifficultyBlocks;
	}

	/// Gets the total importance for the block chain described by \a config.
	constexpr Importance GetTotalImportance(const BlockChainConfiguration& config) {
		return Importance(config.TotalChainBalance.xem().unwrap());
	}

	/// Loads plugin configuration for plugin named \a pluginName from \a config.
	template<typename T>
	T LoadPluginConfiguration(const BlockChainConfiguration& config, const std::string& pluginName) {
		auto iter = config.Plugins.find(pluginName);
		if (config.Plugins.cend() == iter)
			CATAPULT_THROW_AND_LOG_1(utils::property_not_found_error, "plugin configuration not found", pluginName);

		return T::LoadFromBag(iter->second);
	}
}}
