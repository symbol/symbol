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
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/state/MosaicEntry.h"
#include "catapult/state/RootNamespace.h"
#include "catapult/utils/Hashers.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace catapult { namespace utils { class ConfigurationBag; } }

namespace catapult { namespace tools { namespace nemgen {

	/// Mosaic seed.
	struct MosaicSeed {
		/// Mosaic name.
		std::string Name;

		/// Mosaic amount.
		catapult::Amount Amount;
	};

	/// Mapping of namespace ids to human readable names.
	using NamespaceIdToNameMap = std::unordered_map<NamespaceId, std::string, utils::BaseValueHasher<NamespaceId>>;

	/// Mapping of namespace ids to root namespaces.
	using NamespaceIdToRootNamespaceMap = std::unordered_map<NamespaceId, state::RootNamespace, utils::BaseValueHasher<NamespaceId>>;

	/// Mapping of mosaic names to mosaic entries.
	using MosaicNameToMosaicEntryMap = std::vector<std::pair<std::string, state::MosaicEntry>>;

	/// Mapping of addresses to mosaic seeds.
	using AddressToMosaicSeedsMap = std::vector<std::pair<std::string, std::vector<MosaicSeed>>>;

	/// Nemesis configuration.
	struct NemesisConfiguration {
	public:
		/// Block chain network identifier.
		model::NetworkIdentifier NetworkIdentifier;

		/// Nemesis generation hash seed.
		GenerationHashSeed NemesisGenerationHashSeed;

		/// Nemesis signer private key.
		std::string NemesisSignerPrivateKey;

		/// Cpp file header.
		std::string CppFileHeader;

		/// Cpp file path.
		std::string CppFile;

		/// Binary destination directory.
		std::string BinDirectory;

		/// Map containing all namespace names.
		NamespaceIdToNameMap NamespaceNames;

		/// Map containing all root namespaces.
		NamespaceIdToRootNamespaceMap RootNamespaces;

		/// Map containing all mosaic entries.
		MosaicNameToMosaicEntryMap MosaicEntries;

		/// Map of nemesis account addresses to mosaic seeds.
		AddressToMosaicSeedsMap NemesisAddressToMosaicSeeds;

		/// Additional transactions directory.
		std::string TransactionsDirectory;

	public:
		/// Loads a nemesis configuration from \a bag.
		static NemesisConfiguration LoadFromBag(const utils::ConfigurationBag& bag);
	};

	/// Gets the nemesis signer address from \a config.
	Address GetNemesisSignerAddress(const NemesisConfiguration& config);
}}}
