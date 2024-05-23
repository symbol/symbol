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

#pragma once
#include "HeightDependentAddress.h"
#include "NetworkInfo.h"
#include "catapult/types.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/FileSize.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/TimeSpan.h"
#include <stdint.h>
#include <unordered_map>

namespace catapult {
namespace model {

    /// Blockchain configuration settings.
    struct BlockchainConfiguration {
    public:
        /// Blockchain network.
        NetworkInfo Network;

        /// \c true if blockchain should calculate state hashes so that state is fully verifiable at each block.
        bool EnableVerifiableState;

        /// \c true if blockchain should calculate receipts so that state changes are fully verifiable at each block.
        bool EnableVerifiableReceipts;

        /// Mosaic id used as primary chain currency.
        MosaicId CurrencyMosaicId;

        /// Mosaic id used to provide harvesting ability.
        MosaicId HarvestingMosaicId;

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

        /// Percentage of importance resulting from fee generation and beneficiary usage.
        uint8_t ImportanceActivityPercentage;

        /// Maximum number of blocks that can be rolled back.
        uint32_t MaxRollbackBlocks;

        /// Maximum number of blocks to use in a difficulty calculation.
        uint32_t MaxDifficultyBlocks;

        /// Default multiplier to use for dynamic fees.
        BlockFeeMultiplier DefaultDynamicFeeMultiplier;

        /// Maximum lifetime a transaction can have before it expires.
        utils::TimeSpan MaxTransactionLifetime;

        /// Maximum future time of a block that can be accepted.
        utils::TimeSpan MaxBlockFutureTime;

        /// Initial currency atomic units available in the network.
        Amount InitialCurrencyAtomicUnits;

        /// Maximum atomic units (total-supply * 10 ^ divisibility) of a mosaic allowed in the network.
        Amount MaxMosaicAtomicUnits;

        /// Total whole importance units available in the network.
        Importance TotalChainImportance;

        /// Minimum number of harvesting mosaic atomic units needed for an account to be eligible for harvesting.
        Amount MinHarvesterBalance;

        /// Maximum number of harvesting mosaic atomic units needed for an account to be eligible for harvesting.
        Amount MaxHarvesterBalance;

        /// Minimum number of harvesting mosaic atomic units needed for an account to be eligible for voting.
        Amount MinVoterBalance;

        /// Number of blocks that should be treated as a group for voting set purposes.
        /// \note Voting sets will only be calculated at blocks that are multiples of this grouping number.
        uint64_t VotingSetGrouping;

        /// Maximum number of voting keys that can be registered at once per account.
        uint8_t MaxVotingKeysPerAccount;

        /// Minimum number of finalization rounds for which voting key can be registered.
        uint32_t MinVotingKeyLifetime;

        /// Maximum number of finalization rounds for which voting key can be registered.
        uint32_t MaxVotingKeyLifetime;

        /// Percentage of the harvested fee that is collected by the beneficiary account.
        uint8_t HarvestBeneficiaryPercentage;

        /// Percentage of the harvested fee that is collected by the network.
        uint8_t HarvestNetworkPercentage;

        /// Address of the harvest network fee sink account (V1).
        Address HarvestNetworkFeeSinkAddressV1;

        /// Address of the harvest network fee sink account (latest).
        Address HarvestNetworkFeeSinkAddress;

        /// Maximum number of transactions per block.
        uint32_t MaxTransactionsPerBlock;

    public:
        /// Fork heights configuration.
        struct ForkHeights {
            /// Height of fork to fix TotalVotingBalance calculation.
            Height TotalVotingBalanceCalculationFix;

            /// Height of fork at which to reissue the treasury.
            Height TreasuryReissuance;

            /// Height of fork at which aggregate transaction hash is strictly enforced.
            Height StrictAggregateTransactionHash;

            /// Heights at which the secret lock uniqueness requirement is violated.
            /// \note This is required to allow mainnet to sync from scratch due to a (since fixed) bug.
            std::unordered_set<Height, utils::BaseValueHasher<Height>> SkipSecretLockUniquenessChecks;

            /// Heights at which secret lock expiration should be ignored (and locked value burned).
            /// \note This is required to allow mainnet to sync from scratch due to a (since fixed) bug.
            std::unordered_set<Height, utils::BaseValueHasher<Height>> SkipSecretLockExpirations;

            /// Heights at which secret lock expiration should be forced (and locked value created).
            /// \note This is required to allow mainnet to sync from scratch due to a (since fixed) bug.
            std::unordered_set<Height, utils::BaseValueHasher<Height>> ForceSecretLockExpirations;
        };

        /// Fork heights.
        BlockchainConfiguration::ForkHeights ForkHeights;

    public:
        /// Signatures of transactions allowed in the treasury reissuance block (preferred).
        /// \note These are allowed to involve the nemesis account after the nemesis block.
        std::vector<Signature> TreasuryReissuanceTransactionSignatures;

        /// Map of hashes of aggregate transactions to corrupted aggregate transaction hashes.
        std::unordered_map<Hash256, Hash256, utils::ArrayHasher<Hash256>> KnownCorruptAggregateTransactionHashesMap;

        /// Unparsed map of plugin configuration.
        std::unordered_map<std::string, utils::ConfigurationBag> Plugins;

    private:
        BlockchainConfiguration() = default;

    public:
        /// Creates an uninitialized blockchain configuration.
        static BlockchainConfiguration Uninitialized();

        /// Loads a blockchain configuration from \a bag.
        static BlockchainConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
    };

    /// Gets the unresolved currency mosaic id from \a config.
    UnresolvedMosaicId GetUnresolvedCurrencyMosaicId(const BlockchainConfiguration& config);

    /// Gets the harvest network fee sink address from \a config.
    HeightDependentAddress GetHarvestNetworkFeeSinkAddress(const BlockchainConfiguration& config);

    /// Calculates the duration of time that expired transactions should be cached for the blockchain described by \a config.
    utils::TimeSpan CalculateTransactionCacheDuration(const BlockchainConfiguration& config);

    /// Loads plugin configuration for plugin named \a pluginName from \a config.
    template <typename T>
    T LoadPluginConfiguration(const BlockchainConfiguration& config, const std::string& pluginName)
    {
        auto iter = config.Plugins.find(pluginName);
        if (config.Plugins.cend() == iter)
            CATAPULT_THROW_AND_LOG_1(utils::property_not_found_error, "plugin configuration not found", pluginName);

        return T::LoadFromBag(iter->second);
    }
}
}
