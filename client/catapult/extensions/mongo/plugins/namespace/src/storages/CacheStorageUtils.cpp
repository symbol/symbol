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

#include "CacheStorageUtils.h"
#include "src/mappers/MosaicDescriptor.h"
#include "src/mappers/NamespaceDescriptor.h"
#include "plugins/txes/namespace/src/state/MosaicHistory.h"
#include "plugins/txes/namespace/src/state/RootNamespaceHistory.h"
#include "catapult/model/Address.h"

namespace catapult { namespace mongo { namespace plugins {

	std::vector<MosaicDescriptor> MosaicDescriptorsFromHistory(const state::MosaicHistory& history) {
		std::vector<MosaicDescriptor> descriptors;
		uint32_t index = 0;
		for (const auto& mosaicEntry : history) {
			auto isActive = index == history.historyDepth() - 1;
			auto pEntry = std::shared_ptr<const state::MosaicEntry>(&mosaicEntry, [](auto) {});
			descriptors.emplace_back(pEntry, index, isActive);
			++index;
		}

		return descriptors;
	}

	std::vector<NamespaceDescriptor> NamespaceDescriptorsFromHistory(
			const state::RootNamespaceHistory& history,
			model::NetworkIdentifier networkIdentifier) {
		std::vector<NamespaceDescriptor> descriptors;
		uint32_t index = 0;
		state::Namespace::Path path;
		path.push_back(history.id());
		for (const auto& rootNamespaceEntry : history) {
			auto isActive = index == history.historyDepth() - 1;
			auto address = model::PublicKeyToAddress(rootNamespaceEntry.owner(), networkIdentifier);
			auto pRoot = std::shared_ptr<const state::RootNamespace>(&rootNamespaceEntry, [](auto) {});
			descriptors.emplace_back(path, pRoot, address, index, isActive);
			for (const auto& pair : rootNamespaceEntry.children())
				descriptors.emplace_back(pair.second, pRoot, address, index, isActive);

			++index;
		}

		return descriptors;
	}
}}}
