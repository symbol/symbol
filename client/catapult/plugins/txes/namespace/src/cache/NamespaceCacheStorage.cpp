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

#include "NamespaceCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include <map>
#include <vector>

namespace catapult { namespace cache {

	namespace {
		void SaveChildren(io::OutputStream& output, const state::RootNamespace::Children& children) {
			io::Write64(output, children.size());
			for (const auto& child : children) {
				const auto& path = child.second;

				// don't write the first part of the path (the root id) because it is redundant
				auto i = 1u;
				for (; i < path.size(); ++i)
					io::Write(output, path[i]);

				// pad the storage so that all children have a fixed size in the storage
				for (; i < path.capacity(); ++i)
					io::Write(output, NamespaceId());
			}
		}
	}

	void NamespaceCacheStorage::Save(const StorageType& element, io::OutputStream& output) {
		const auto& history = element.second;
		if (0 == history.historyDepth())
			CATAPULT_THROW_RUNTIME_ERROR_1("cannot save empty namespace history", history.id());

		io::Write(output, history.id());
		io::Write64(output, history.historyDepth());

		const Key *pLastOwner = nullptr;
		for (const auto& root : history) {
			io::Write(output, root.owner());
			io::Write(output, root.lifetime().Start);
			io::Write(output, root.lifetime().End);

			if (pLastOwner && *pLastOwner == root.owner()) {
				// shared owner, don't rewrite children
				io::Write64(output, 0);
				continue;
			}

			pLastOwner = &root.owner();
			SaveChildren(output, root.children());
		}
	}

	namespace {
		struct Header {
			catapult::NamespaceId Id;
			uint64_t HistoryDepth;
		};

		Header ReadHeader(io::InputStream& input) {
			Header header;
			header.Id = io::Read<NamespaceId>(input);
			header.HistoryDepth = io::Read64(input);

			if (0 == header.HistoryDepth)
				CATAPULT_THROW_RUNTIME_ERROR_1("namespace history in storage is empty", header.Id);

			return header;
		}

		state::Namespace::Path LoadPath(io::InputStream& input, NamespaceId rootId) {
			state::Namespace::Path path;
			path.push_back(rootId);
			for (auto i = 0u; i < path.capacity() - 1; ++i) {
				NamespaceId idPart;
				io::Read(input, idPart);
				if (NamespaceId() == idPart)
					continue;

				path.push_back(idPart);
			}

			return path;
		}

		using NamespacePathsGroupedByDepth = std::map<size_t, std::vector<state::Namespace::Path>>;

		NamespacePathsGroupedByDepth LoadChildren(io::InputStream& input, NamespaceId rootId, size_t numChildren) {
			// load all paths and sort them by size
			NamespacePathsGroupedByDepth groupedPaths;
			for (auto i = 0u; i < numChildren; ++i) {
				auto path = LoadPath(input, rootId);
				groupedPaths[path.size()].push_back(path);
			}

			return groupedPaths;
		}

		void LoadChildren(io::InputStream& input, NamespaceId rootId, size_t numChildren, NamespaceCacheDelta& cacheDelta) {
			// load all paths with smaller sizes before larger sizes
			auto groupedPaths = LoadChildren(input, rootId, numChildren);
			for (const auto& pair : groupedPaths) {
				for (const auto& path : pair.second)
					cacheDelta.insert(state::Namespace(path));
			}
		}
	}

	state::RootNamespaceHistory NamespaceCacheStorage::Load(io::InputStream& input) {
		auto header = ReadHeader(input);
		state::RootNamespaceHistory history(header.Id);

		for (auto i = 0u; i < header.HistoryDepth; ++i) {
			Key owner;
			input.read(owner);
			auto lifetimeStart = io::Read<Height>(input);
			auto lifetimeEnd = io::Read<Height>(input);
			history.push_back(owner, state::NamespaceLifetime(lifetimeStart, lifetimeEnd));

			auto numChildren = io::Read64(input);
			auto groupedPaths = LoadChildren(input, header.Id, numChildren);

			auto& currentRoot = history.back();
			for (const auto& pair : groupedPaths) {
				for (const auto& path : pair.second)
					currentRoot.add(state::Namespace(path));
			}
		}

		return history;
	}

	void NamespaceCacheStorage::LoadInto(io::InputStream& input, DestinationType& cacheDelta) {
		auto header = ReadHeader(input);

		for (auto i = 0u; i < header.HistoryDepth; ++i) {
			Key owner;
			input.read(owner);
			auto lifetimeStart = io::Read<Height>(input);
			auto lifetimeEnd = io::Read<Height>(input);
			cacheDelta.insert(state::RootNamespace(header.Id, owner, state::NamespaceLifetime(lifetimeStart, lifetimeEnd)));

			auto numChildren = io::Read64(input);
			LoadChildren(input, header.Id, numChildren, cacheDelta);
		}
	}
}}
