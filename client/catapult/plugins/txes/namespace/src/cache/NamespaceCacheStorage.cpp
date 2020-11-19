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

#include "NamespaceCacheStorage.h"
#include "NamespaceCacheDelta.h"

namespace catapult { namespace cache {

	namespace {
		using ChildNamespaceData = state::RootNamespace::ChildNamespaceData;

		struct PathsComparator {
		public:
			bool operator()(const ChildNamespaceData& lhs, const ChildNamespaceData& rhs) const {
				return operator()(lhs.Path, rhs.Path);
			}

		private:
			bool operator()(const state::Namespace::Path& lhs, const state::Namespace::Path& rhs) const {
				return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
			}
		};

		using SortedNamespaceByPathMap = std::map<ChildNamespaceData, NamespaceId, PathsComparator>;

		SortedNamespaceByPathMap SortChildren(const state::RootNamespace::Children& children) {
			SortedNamespaceByPathMap sortedMap;
			for (const auto& child : children)
				sortedMap.emplace(child.second, child.first);

			return sortedMap;
		}
	}

	void NamespaceCacheStorage::LoadInto(const ValueType& history, DestinationType& cacheDelta) {
		for (const auto& rootNamespace : history) {
			cacheDelta.insert(rootNamespace);
			cacheDelta.setAlias(rootNamespace.id(), rootNamespace.alias(rootNamespace.id()));

			auto childrenMap = SortChildren(rootNamespace.children());
			for (const auto& pair : childrenMap) {
				if (!cacheDelta.contains(pair.second)) {
					cacheDelta.insert(state::Namespace(pair.first.Path));
					cacheDelta.setAlias(pair.second, pair.first.Alias);
				}
			}
		}
	}

	void NamespaceCacheStorage::Purge(const ValueType& history, DestinationType& cacheDelta) {
		while (cacheDelta.contains(history.id())) {
			auto childrenMap = SortChildren(history.back().children());
			for (auto iter = childrenMap.crbegin(); childrenMap.crend() != iter; ++iter) {
				if (cacheDelta.contains(iter->second))
					cacheDelta.remove(iter->second);
			}

			cacheDelta.remove(history.id());
		}
	}
}}
