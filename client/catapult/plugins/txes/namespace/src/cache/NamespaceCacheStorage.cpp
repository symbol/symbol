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

	void NamespaceCacheStorage::Save(const ValueType& value, io::OutputStream& output) {
		const auto& history = value.second;
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

		void LoadChildren(io::InputStream& input, NamespaceCacheDelta& cacheDelta, NamespaceId rootId, size_t numChildren) {
			// load all paths and sort them by size
			std::map<size_t, std::vector<state::Namespace::Path>> paths;
			for (auto i = 0u; i < numChildren; ++i) {
				auto path = LoadPath(input, rootId);
				paths[path.size()].push_back(path);
			}

			// load all paths with smaller sizes before larger sizes
			for (const auto& pair : paths) {
				for (const auto& path : pair.second)
					cacheDelta.insert(state::Namespace(path));
			}
		}
	}

	void NamespaceCacheStorage::Load(io::InputStream& input, DestinationType& cacheDelta) {
		// - read header
		auto id = io::Read<NamespaceId>(input);
		auto historyDepth = io::Read64(input);

		if (0 == historyDepth)
			CATAPULT_THROW_RUNTIME_ERROR_1("namespace history in storage is empty", id);

		for (auto i = 0u; i < historyDepth; ++i) {
			Key owner;
			input.read(owner);
			auto lifetimeStart = io::Read<Height>(input);
			auto lifetimeEnd = io::Read<Height>(input);
			cacheDelta.insert(state::RootNamespace(id, owner, state::NamespaceLifetime(lifetimeStart, lifetimeEnd)));

			auto numChildren = io::Read64(input);
			LoadChildren(input, cacheDelta, id, numChildren);
		}
	}
}}
