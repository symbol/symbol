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

#include "MongoNamespaceCacheStorageUtils.h"
#include "src/mappers/NamespaceDescriptor.h"
#include "plugins/txes/namespace/src/state/RootNamespaceHistory.h"

namespace catapult { namespace mongo { namespace plugins {

	std::vector<NamespaceDescriptor> NamespaceDescriptorsFromHistory(const state::RootNamespaceHistory& history) {
		std::vector<NamespaceDescriptor> descriptors;
		uint32_t index = 0;
		state::Namespace::Path path;
		path.push_back(history.id());
		for (const auto& rootNamespace : history) {
			auto isActive = index == history.historyDepth() - 1;
			auto address = rootNamespace.ownerAddress();
			auto pRoot = std::shared_ptr<const state::RootNamespace>(&rootNamespace, [](auto) {});
			auto rootAlias = rootNamespace.alias(rootNamespace.id());
			descriptors.emplace_back(path, rootAlias, pRoot, address, index, isActive);
			for (const auto& pair : rootNamespace.children())
				descriptors.emplace_back(pair.second.Path, pair.second.Alias, pRoot, address, index, isActive);

			++index;
		}

		return descriptors;
	}
}}}
