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

#include "ValidateConfiguration.h"
#include "CatapultConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/HexParser.h"

namespace catapult { namespace config {

#define CATAPULT_THROW_VALIDATION_ERROR(MESSAGE) CATAPULT_THROW_AND_LOG_0(utils::property_malformed_error, MESSAGE)

	namespace {
		void ValidateConfiguration(const model::BlockChainConfiguration& config) {
			if (2 * config.ImportanceGrouping <= config.MaxRollbackBlocks) {
				std::ostringstream out;
				out
						<< "ImportanceGrouping (" << config.ImportanceGrouping << ") must be greater than MaxRollbackBlocks ("
						<< config.MaxRollbackBlocks << ") / 2";
				CATAPULT_THROW_VALIDATION_ERROR(out.str().c_str());
			}

			if (100u < config.HarvestBeneficiaryPercentage + config.HarvestNetworkPercentage) {
				std::ostringstream out;
				out
						<< "HarvestBeneficiaryPercentage (" << config.HarvestBeneficiaryPercentage << ") plus HarvestNetworkPercentage ("
						<< config.HarvestNetworkPercentage << ") must not be greater than 100";
				CATAPULT_THROW_VALIDATION_ERROR(out.str().c_str());
			}

			if (99u < config.ImportanceActivityPercentage) {
				std::ostringstream out;
				out << "ImportanceActivityPercentage (" << config.ImportanceActivityPercentage << ") must not be greater than 99";
				CATAPULT_THROW_VALIDATION_ERROR(out.str().c_str());
			}

			if (0 != config.VotingSetGrouping % config.ImportanceGrouping) {
				std::ostringstream out;
				out
						<< "VotingSetGrouping (" << config.VotingSetGrouping << ") must be multiple of ImportanceGrouping ("
						<< config.ImportanceGrouping << ")";
				CATAPULT_THROW_VALIDATION_ERROR(out.str().c_str());
			}
		}

		void ValidateConfiguration(
				const model::BlockChainConfiguration& blockChainConfig,
				const config::InflationConfiguration& inflationConfig) {
			auto totalInflation = inflationConfig.InflationCalculator.sumAll();
			if (!totalInflation.second)
				CATAPULT_THROW_VALIDATION_ERROR("total currency inflation could not be calculated");

			auto totalCurrency = blockChainConfig.InitialCurrencyAtomicUnits + totalInflation.first;
			if (blockChainConfig.InitialCurrencyAtomicUnits > totalCurrency || totalCurrency > blockChainConfig.MaxMosaicAtomicUnits) {
				std::ostringstream out;
				out
						<< "sum of InitialCurrencyAtomicUnits (" << blockChainConfig.InitialCurrencyAtomicUnits << ") and inflation ("
						<< totalInflation.first << ") must not exceed MaxMosaicAtomicUnits ("
						<< blockChainConfig.MaxMosaicAtomicUnits << ")";
				CATAPULT_THROW_VALIDATION_ERROR(out.str().c_str());
			}
		}

		void ValidateConfiguration(const config::NodeConfiguration& config) {
			auto maxWriteBatchSize = config.CacheDatabase.MaxWriteBatchSize;
			if (utils::FileSize() != maxWriteBatchSize && maxWriteBatchSize < utils::FileSize::FromKilobytes(100)) {
				std::ostringstream out;
				out << "MaxWriteBatchSize (" << maxWriteBatchSize << ") must be unset or at least 100KB";
				CATAPULT_THROW_VALIDATION_ERROR(out.str().c_str());
			}
		}
	}

	void ValidateConfiguration(const CatapultConfiguration& config) {
		ValidateConfiguration(config.BlockChain);
		ValidateConfiguration(config.BlockChain, config.Inflation);
		ValidateConfiguration(config.Node);
	}

#undef CATAPULT_THROW_VALIDATION_ERROR
}}
