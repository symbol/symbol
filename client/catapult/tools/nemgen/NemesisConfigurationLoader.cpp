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

#include "NemesisConfigurationLoader.h"
#include "catapult/model/Address.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/HexFormatter.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace tools { namespace nemgen {

	namespace {
		template<typename TIdentifier>
		void OutputName(const std::string& name, TIdentifier id) {
			CATAPULT_LOG(debug) << " - " << name << " (" << utils::HexFormat(id) << ")";
		}

		void LogNamespaces(const NemesisConfiguration& config) {
			CATAPULT_LOG(debug) << "Namespace Summary";
			for (const auto& pair : config.RootNamespaces) {
				const auto& root = pair.second;
				const auto& name = config.NamespaceNames.at(root.id());
				OutputName(name, root.id());
				CATAPULT_LOG(debug) << " - Owner: " << root.ownerAddress();
				CATAPULT_LOG(debug) << " - Start Height: " << root.lifetime().Start;
				CATAPULT_LOG(debug) << " - End Height: " << root.lifetime().End;
				if (!root.empty()) {
					CATAPULT_LOG(debug) << " - Children:";
					for (const auto & childPair : root.children()) {
						const auto& childName = config.NamespaceNames.at(childPair.first);
						CATAPULT_LOG(debug) << " - - " << childName << " (" << utils::HexFormat(childPair.first) << ")";
					}
				}

				CATAPULT_LOG(debug);
			}
		}

		bool LogMosaicDefinitions(const NemesisConfiguration& config) {
			CATAPULT_LOG(debug) << "Mosaic Summary";
			std::unordered_set<MosaicId, utils::BaseValueHasher<MosaicId>> mosaicIds;
			for (const auto& pair : config.MosaicEntries) {
				const std::string& name = pair.first;
				auto id = pair.second.mosaicId();
				const auto& mosaicEntry = pair.second;
				const auto& definition = mosaicEntry.definition();
				const auto& properties = definition.properties();
				OutputName(name, id);
				CATAPULT_LOG(debug)
						<< " - Owner: " << definition.ownerAddress() << std::endl
						<< " - Supply: " << mosaicEntry.supply() << std::endl
						<< " - Divisibility: " << static_cast<uint32_t>(properties.divisibility()) << std::endl
						<< " - Duration: " << properties.duration() << " blocks (0 = eternal)" << std::endl
						<< " - IsTransferable: " << properties.is(model::MosaicFlags::Transferable) << std::endl
						<< " - IsSupplyMutable: " << properties.is(model::MosaicFlags::Supply_Mutable) << std::endl
						<< " - IsRestrictable: " << properties.is(model::MosaicFlags::Restrictable) << std::endl
						<< std::endl;

				if (!mosaicIds.insert(id).second) {
					CATAPULT_LOG(warning) << "mosaic " << name << " does not have a unique id";
					return false;
				}
			}

			return true;
		}

		bool LogMosaicDistribution(const NemesisConfiguration& config) {
			CATAPULT_LOG(debug) << "Nemesis Seed Amounts";
			for (const auto& addressMosaicSeedsPair : config.NemesisAddressToMosaicSeeds) {
				const auto& address = addressMosaicSeedsPair.first;
				CATAPULT_LOG(debug) << " - " << address;
				if (!model::IsValidEncodedAddress(address, config.NetworkIdentifier)) {
					CATAPULT_LOG(warning) << "address " << address << " is invalid";
					return false;
				}

				for (const auto& seed : addressMosaicSeedsPair.second)
					CATAPULT_LOG(debug) << " - - " << seed.Name << ": " << seed.Amount;
			}

			return true;
		}
	}

	NemesisConfiguration LoadNemesisConfiguration(const std::string& configPath) {
		if (!boost::filesystem::exists(configPath)) {
			auto message = "aborting load due to missing configuration file";
			CATAPULT_LOG(fatal) << message << ": " << configPath;
			CATAPULT_THROW_EXCEPTION(catapult_runtime_error(message));
		}

		CATAPULT_LOG(info) << "loading nemesis configuration from " << configPath;
		return NemesisConfiguration::LoadFromBag(utils::ConfigurationBag::FromPath(configPath));
	}

	bool LogAndValidateNemesisConfiguration(const NemesisConfiguration& config) {
		CATAPULT_LOG(debug) << "--- Nemesis Configuration ---";
		CATAPULT_LOG(debug) << "Network            : " << config.NetworkIdentifier;
		CATAPULT_LOG(debug) << "Gen Hash Seed      : " << config.NemesisGenerationHashSeed;
		CATAPULT_LOG(debug) << "Nemesis Private Key: " << config.NemesisSignerPrivateKey;
		CATAPULT_LOG(debug) << "Txes Directory     : " << config.TransactionsDirectory;
		CATAPULT_LOG(debug) << "Cpp File Header    : " << config.CppFileHeader;
		CATAPULT_LOG(debug) << "Cpp File           : " << config.CppFile;
		CATAPULT_LOG(debug) << "Bin Directory      : " << config.BinDirectory;

		// - namespaces
		LogNamespaces(config);

		// - mosaic definitions and distribution
		return LogMosaicDefinitions(config) && LogMosaicDistribution(config);
	}
}}}
