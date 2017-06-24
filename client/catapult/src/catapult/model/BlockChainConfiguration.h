#pragma once
#include "NetworkInfo.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/FileSize.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/types.h"
#include <unordered_map>
#include <stdint.h>

namespace catapult { namespace model {

	/// Block chain configuration settings.
	struct BlockChainConfiguration {
	public:
		/// The block chain network.
		model::NetworkInfo Network;

		/// The targeted time between blocks.
		utils::TimeSpan BlockGenerationTargetTime;

		/// The smoothing factor in thousandths.
		/// If this value is non-zero, the network will be biased in favor of evenly spaced blocks.
		/// The higher the value, the more biased the network is.
		/// \note This can lower security because it will increase the influence of time relative to importance.
		uint32_t BlockTimeSmoothingFactor;

		/// The number of blocks that should be treated as a group for importance purposes.
		/// \note Importances will only be calculated at blocks that are multiples of this grouping number.
		uint64_t ImportanceGrouping;

		/// The maximum number of blocks that can be rolled back.
		uint32_t MaxRollbackBlocks;

		/// The maximum number of blocks to use in a difficulty calculation.
		uint32_t MaxDifficultyBlocks;

		/// The maximum lifetime a transaction can have before it expires.
		utils::TimeSpan MaxTransactionLifetime;

		/// The maximum future time of a block that can be accepted.
		utils::TimeSpan MaxBlockFutureTime;

		/// The total number of micro XEM available in the network.
		Amount TotalChainBalance;

		/// The minimum number of micro XEM needed for an account to eligible for harvesting.
		Amount MinHarvesterBalance;

		/// The number of blocks between cache pruning.
		uint32_t BlockPruneInterval;

		/// The maximum number of transactions per block.
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

	/// Loads plugin configuration for plugin named \a pluginName from \a config.
	template<typename T>
	T LoadPluginConfiguration(const BlockChainConfiguration& config, const std::string& pluginName) {
		auto iter = config.Plugins.find(pluginName);
		if (config.Plugins.cend() == iter)
			CATAPULT_THROW_AND_LOG_1(utils::property_not_found_error, "plugin configuration not found", pluginName);

		return T::LoadFromBag(iter->second);
	}
}}
