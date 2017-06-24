#pragma once
#include "catapult/model/NetworkInfo.h"
#include "catapult/state/MosaicEntry.h"
#include "catapult/state/RootNamespace.h"
#include "catapult/utils/Hashers.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace tools { namespace nemgen {

	/// A mosaic seed.
	struct MosaicSeed {
		/// The mosaic name.
		std::string Name;

		/// The mosaic amount.
		catapult::Amount Amount;
	};

	/// Mapping of namespace ids to human readable names.
	using NamespaceIdToNameMap = std::unordered_map<NamespaceId, std::string, utils::BaseValueHasher<NamespaceId>>;

	/// Mapping of namespace ids to root namespaces.
	using NamespaceIdToRootNamespaceMap = std::unordered_map<NamespaceId, state::RootNamespace, utils::BaseValueHasher<NamespaceId>>;

	/// Mapping of mosaic names to mosaic entries.
	using MosaicNameToMosaicEntryMap = std::unordered_map<std::string, state::MosaicEntry>;

	/// Mapping of addresses to mosaic seeds.
	using AddressToMosaicSeedsMap = std::unordered_map<std::string, std::vector<MosaicSeed>>;

	/// Nemesis configuration.
	struct NemesisConfiguration {
	public:
		/// The block chain network identifier.
		model::NetworkIdentifier NetworkIdentifier;

		/// The nemesis generation hash.
		std::string NemesisGenerationHash;

		/// The nemesis signer private key.
		std::string NemesisSignerPrivateKey;

		/// The cpp file path.
		std::string CppFile;

		/// The binary destination directory.
		std::string BinDirectory;

		/// Map containing all namespace names.
		NamespaceIdToNameMap NamespaceNames;

		/// Map containing all root namespaces.
		NamespaceIdToRootNamespaceMap RootNamespaces;

		/// Map containing all mosaic entries.
		MosaicNameToMosaicEntryMap MosaicEntries;

		/// Map of nemesis account addresses to mosaic seeds.
		AddressToMosaicSeedsMap NemesisAddressToMosaicSeeds;

	public:
		/// Loads a nemesis configuration from \a bag.
		static NemesisConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};
}}}
