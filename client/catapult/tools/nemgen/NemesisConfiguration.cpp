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

#include "NemesisConfiguration.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/extensions/IdGenerator.h"
#include "catapult/model/Address.h"
#include "catapult/model/MosaicIdGenerator.h"
#include "catapult/model/NamespaceIdGenerator.h"
#include "catapult/state/Namespace.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace tools { namespace nemgen {

	namespace {
		constexpr auto Namespace_Section_Prefix = "namespace>";
		constexpr auto Mosaic_Section_Prefix = "mosaic>";
		constexpr auto Distribution_Section_Prefix = "distribution>";
		constexpr size_t Num_Namespace_Properties = 1; // duration
		constexpr size_t Num_Mosaic_Properties = 6; // divisibility, duration, supply, 3 flags

		template<typename TContainer>
		auto FindByKey(TContainer& pairContainer, const typename TContainer::value_type::first_type& key) {
			return std::find_if(pairContainer.begin(), pairContainer.end(), [&key](const auto& pair) {
				return key == pair.first;
			});
		}

		void Merge(
				AddressToMosaicSeedsMap& aggregateMap,
				const std::string& mosaicName,
				const std::vector<std::pair<std::string, uint64_t>>& addressToAmountMap) {
			for (const auto& addressAmountPair : addressToAmountMap) {
				const auto& address = addressAmountPair.first;
				auto iter = FindByKey(aggregateMap, address);
				if (aggregateMap.end() == iter) {
					aggregateMap.emplace_back(address, std::vector<MosaicSeed>());
					iter = aggregateMap.end() - 1;
				}

				iter->second.push_back({ mosaicName, Amount(addressAmountPair.second) });
			}
		}

		auto IsRoot(const std::string& namespaceName) {
			return std::string::npos == namespaceName.find('.');
		}

		auto CreateRoot(const utils::ConfigurationBag& bag, const Address& owner, const std::string& namespaceName) {
			const std::string section = Namespace_Section_Prefix + namespaceName;
			auto duration = bag.get<uint64_t>(utils::ConfigurationKey(section.c_str(), "duration"));
			auto id = model::GenerateRootNamespaceId(namespaceName);
			auto endHeight = 0 == duration ? Height(std::numeric_limits<BlockDuration::ValueType>::max()) : Height(duration + 1);
			return state::RootNamespace(id, owner, state::NamespaceLifetime(Height(1), endHeight));
		}

		auto ToMosaicEntry(const state::MosaicDefinition& definition, MosaicNonce mosaicNonce, Amount supply) {
			auto entry = state::MosaicEntry(model::GenerateMosaicId(definition.ownerAddress(), mosaicNonce), definition);
			entry.increaseSupply(supply);
			return entry;
		}

		auto CreateMosaicEntry(
				const utils::ConfigurationBag& bag,
				const Address& owner,
				const std::string& mosaicName,
				MosaicNonce mosaicNonce) {
			const std::string section = Mosaic_Section_Prefix + mosaicName;
			auto makeKey = [&section](const auto* name) {
				return utils::ConfigurationKey(section.c_str(), name);
			};

			auto supply = Amount(bag.get<uint64_t>(makeKey("supply")));
			auto divisibility = bag.get<uint8_t>(makeKey("divisibility"));
			auto duration = BlockDuration(bag.get<uint64_t>(makeKey("duration")));

			auto flags = model::MosaicFlags::None;
			if (bag.get<bool>(makeKey("isTransferable")))
				flags |= model::MosaicFlags::Transferable;

			if (bag.get<bool>(makeKey("isSupplyMutable")))
				flags |= model::MosaicFlags::Supply_Mutable;

			if (bag.get<bool>(makeKey("isRestrictable")))
				flags |= model::MosaicFlags::Restrictable;

			state::MosaicDefinition definition(Height(1), owner, 1, model::MosaicProperties(flags, divisibility, duration));
			return ToMosaicEntry(definition, mosaicNonce, supply);
		}

		size_t LoadNamespaces(const utils::ConfigurationBag& bag, NemesisConfiguration& config, const Address& owner) {
			auto namespaces = bag.getAllOrdered<bool>("namespaces");
			auto numNamespaceProperties = namespaces.size();
			for (const auto& optionalNs : namespaces) {
				const auto& namespaceName = optionalNs.first;
				if (IsRoot(namespaceName)) {
					auto root = CreateRoot(bag, owner, namespaceName);
					numNamespaceProperties += Num_Namespace_Properties;
					if (!optionalNs.second)
						continue;

					auto result = config.RootNamespaces.emplace(root.id(), root);
					if (!result.second)
						CATAPULT_THROW_INVALID_ARGUMENT_1("duplicate root namespace", namespaceName);

					config.NamespaceNames.emplace(root.id(), namespaceName);
					continue;
				}

				if (!optionalNs.second)
					continue;

				auto child = state::Namespace(extensions::GenerateNamespacePath(namespaceName));
				auto rootIter = config.RootNamespaces.find(child.rootId());
				if (config.RootNamespaces.cend() == rootIter)
					CATAPULT_THROW_INVALID_ARGUMENT_1("root namespace not found", namespaceName);

				// note that add will throw if the child is already known
				rootIter->second.add(child);
				config.NamespaceNames.emplace(child.id(), namespaceName);
			}

			return numNamespaceProperties;
		}

		size_t LoadMosaics(const utils::ConfigurationBag& bag, NemesisConfiguration& config, const Address& owner) {
			auto mosaics = bag.getAllOrdered<bool>("mosaics");
			auto numMosaicProperties = mosaics.size();

			uint32_t mosaicNonce = 0;
			for (const auto& optionalMosaic : mosaics) {
				const auto& mosaicName = optionalMosaic.first;

				// - mosaic entry
				auto mosaicEntry = CreateMosaicEntry(bag, owner, mosaicName, MosaicNonce(mosaicNonce));
				numMosaicProperties += Num_Mosaic_Properties;
				++mosaicNonce;

				// - initial distribution
				const std::string section = Distribution_Section_Prefix + mosaicName;
				auto addressToAmountMap = bag.getAllOrdered<uint64_t>(section.c_str());
				numMosaicProperties += addressToAmountMap.size();
				if (!optionalMosaic.second)
					continue;

				// - add information
				if (config.MosaicEntries.cend() != FindByKey(config.MosaicEntries, mosaicName))
					CATAPULT_THROW_RUNTIME_ERROR_1("multiple entries for", mosaicName);

				config.MosaicEntries.emplace_back(mosaicName, mosaicEntry);
				Merge(config.NemesisAddressToMosaicSeeds, mosaicName, addressToAmountMap);
			}

			return numMosaicProperties;
		}
	}

#define LOAD_PROPERTY(SECTION, NAME) utils::LoadIniProperty(bag, SECTION, #NAME, config.NAME)

	NemesisConfiguration NemesisConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		NemesisConfiguration config;

#define LOAD_NEMESIS_PROPERTY(NAME) LOAD_PROPERTY("nemesis", NAME)

		LOAD_NEMESIS_PROPERTY(NetworkIdentifier);
		LOAD_NEMESIS_PROPERTY(NemesisGenerationHashSeed);
		LOAD_NEMESIS_PROPERTY(NemesisSignerPrivateKey);

#undef LOAD_NEMESIS_PROPERTY

#define LOAD_CPP_PROPERTY(NAME) LOAD_PROPERTY("cpp", NAME)

		LOAD_CPP_PROPERTY(CppFileHeader);

#undef LOAD_CPP_PROPERTY

#define LOAD_OUTPUT_PROPERTY(NAME) LOAD_PROPERTY("output", NAME)

		LOAD_OUTPUT_PROPERTY(CppFile);
		LOAD_OUTPUT_PROPERTY(BinDirectory);

#undef LOAD_OUTPUT_PROPERTY

		// the nemesis account owns all namespaces and mosaic definitions in the configuration
		auto ownerPublicKey = crypto::KeyPair::FromString(config.NemesisSignerPrivateKey).publicKey();
		auto owner = model::PublicKeyToAddress(ownerPublicKey, config.NetworkIdentifier);

		// load namespace information
		auto numNamespaceProperties = LoadNamespaces(bag, config, owner);

		// load mosaics information
		auto numMosaicProperties = LoadMosaics(bag, config, owner);

		LOAD_PROPERTY("transactions", TransactionsDirectory);

		utils::VerifyBagSizeLte(bag, 7 + numNamespaceProperties + numMosaicProperties);
		return config;
	}
}}}
