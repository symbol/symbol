#include "NemesisConfiguration.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/extensions/IdGenerator.h"
#include "catapult/model/IdGenerator.h"
#include "catapult/state/Namespace.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace tools { namespace nemgen {

	namespace {
		constexpr const char* Namespace_Section_Prefix = "namespace>";
		constexpr const char* Mosaic_Section_Prefix = "mosaic>";
		constexpr const char* Distribution_Section_Prefix = "distribution>";
		constexpr size_t Num_Namespace_Properties = 1; // duration
		constexpr size_t Num_Mosaic_Properties = 6; // divisibility, duration, supply, 3 flags

		void Merge(
				AddressToMosaicSeedsMap& aggregateMap,
				const std::string& mosaicName,
				const std::unordered_map<std::string, uint64_t>& addressToAmountMap) {
			for (const auto& addressAmountPair : addressToAmountMap) {
				const auto& address = addressAmountPair.first;
				auto iter = aggregateMap.find(address);
				if (aggregateMap.cend() == iter)
					iter = aggregateMap.emplace(address, std::vector<MosaicSeed>()).first;

				iter->second.push_back({ mosaicName, Amount(addressAmountPair.second) });
			}
		}

		auto IsRoot(const std::string& namespaceName) {
			return std::string::npos == namespaceName.find('.');
		}

		auto CreateRoot(const utils::ConfigurationBag& bag, const Key& owner, const std::string& namespaceName) {
			const std::string section = Namespace_Section_Prefix + namespaceName;
			auto duration = bag.get<uint64_t>(utils::ConfigurationKey(section.c_str(), "duration"));
			auto id = model::GenerateRootNamespaceId(namespaceName);
			auto endHeight = 0 == duration ? Height(std::numeric_limits<ArtifactDuration::ValueType>::max()) : Height(duration + 1);
			return state::RootNamespace(id, owner, state::NamespaceLifetime(Height(1), endHeight));
		}

		auto ToMosaicEntry(const std::string& name, const state::MosaicDefinition& definition, Amount supply) {
			auto pos = name.find_last_of(':');
			if (std::string::npos == pos)
				CATAPULT_THROW_INVALID_ARGUMENT_1("invalid mosaic name", name);

			auto namespaceName = name.substr(0, pos);
			auto mosaicName = name.substr(pos + 1, name.size() - pos - 1);
			auto ns = state::Namespace(extensions::GenerateNamespacePath(namespaceName));
			auto entry = state::MosaicEntry(
					ns.id(),
					model::GenerateMosaicId(ns.id(), mosaicName),
					definition);
			entry.increaseSupply(supply);
			return entry;
		}

		auto CreateMosaicEntry(const utils::ConfigurationBag& bag, const Key& owner, const std::string& mosaicName) {
			const std::string section = Mosaic_Section_Prefix + mosaicName;
			auto makeKey = [&section](const auto* name) {
				return utils::ConfigurationKey(section.c_str(), name);
			};

			auto supply = Amount(bag.get<uint64_t>(makeKey("supply")));
			model::MosaicProperties::PropertyValuesContainer values{};
			values[utils::to_underlying_type(model::MosaicPropertyId::Divisibility)] = bag.get<uint8_t>(makeKey("divisibility"));
			values[utils::to_underlying_type(model::MosaicPropertyId::Duration)] = bag.get<uint64_t>(makeKey("duration"));

			model::MosaicFlags flags(model::MosaicFlags::None);
			if (bag.get<bool>(makeKey("isTransferable")))
				flags |= model::MosaicFlags::Transferable;

			if (bag.get<bool>(makeKey("isSupplyMutable")))
				flags |= model::MosaicFlags::Supply_Mutable;

			if (bag.get<bool>(makeKey("isLevyMutable")))
				flags |= model::MosaicFlags::Levy_Mutable;

			values[utils::to_underlying_type(model::MosaicPropertyId::Flags)] = utils::to_underlying_type(flags);
			state::MosaicDefinition definition(Height(1), owner, model::MosaicProperties::FromValues(values));
			return ToMosaicEntry(mosaicName, definition, supply);
		}

		size_t LoadNamespaces(const utils::ConfigurationBag& bag, NemesisConfiguration& config, const Key& owner) {
			auto namespaces = bag.getAll<bool>("namespaces");
			auto numNamespaceProperties = namespaces.size();
			for (const auto& optionalNs : namespaces) {
				const auto& namespaceName = optionalNs.first;
				if (IsRoot(namespaceName)) {
					auto root = CreateRoot(bag, owner, namespaceName);
					numNamespaceProperties += Num_Namespace_Properties;
					if (!optionalNs.second)
						continue;

					auto pair = config.RootNamespaces.emplace(root.id(), root);
					if (!pair.second)
						CATAPULT_THROW_INVALID_ARGUMENT_1("duplicate root namespace", namespaceName);

					config.NamespaceNames.emplace(root.id(), namespaceName);
					continue;
				}

				if (!optionalNs.second)
					continue;

				auto child = state::Namespace(extensions::GenerateNamespacePath(namespaceName));
				auto rootIter = config.RootNamespaces.find(child.rootId());
				if (config.RootNamespaces.cend() == rootIter)
					CATAPULT_THROW_INVALID_ARGUMENT_1("root namespace not found", child.rootId());

				// note that add will throw if the child is already known
				rootIter->second.add(child);
				config.NamespaceNames.emplace(child.id(), namespaceName);
			}

			return numNamespaceProperties;
		}

		size_t LoadMosaics(const utils::ConfigurationBag& bag, NemesisConfiguration& config, const Key& owner) {
			auto mosaics = bag.getAll<bool>("mosaics");
			auto numMosaicProperties = mosaics.size();
			for (const auto& optionalMosaic : mosaics) {
				const auto& mosaicName = optionalMosaic.first;

				// - mosaic entry
				auto mosaicEntry = CreateMosaicEntry(bag, owner, mosaicName);
				numMosaicProperties += Num_Mosaic_Properties;

				// - initial distribution
				const std::string section = Distribution_Section_Prefix + mosaicName;
				auto addressToAmountMap = bag.getAll<uint64_t>(section.c_str());
				numMosaicProperties += addressToAmountMap.size();
				if (!optionalMosaic.second)
					continue;

				// - add information
				config.MosaicEntries.emplace(mosaicName, mosaicEntry);
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
		LOAD_NEMESIS_PROPERTY(NemesisGenerationHash);
		LOAD_NEMESIS_PROPERTY(NemesisSignerPrivateKey);

#undef LOAD_NEMESIS_PROPERTY

#define LOAD_OUTPUT_PROPERTY(NAME) LOAD_PROPERTY("output", NAME)

		LOAD_OUTPUT_PROPERTY(CppFile);
		LOAD_OUTPUT_PROPERTY(BinDirectory);

#undef LOAD_OUTPUT_PROPERTY

		// the nemesis account owns all namespaces and mosaic definitions in the configuration
		auto owner = crypto::KeyPair::FromString(config.NemesisSignerPrivateKey).publicKey();

		// load namespace information
		auto numNamespaceProperties = LoadNamespaces(bag, config, owner);

		// load mosaics information
		auto numMosaicProperties = LoadMosaics(bag, config, owner);

		utils::VerifyBagSizeLte(bag, 5 + numNamespaceProperties + numMosaicProperties);
		return config;
	}
}}}
