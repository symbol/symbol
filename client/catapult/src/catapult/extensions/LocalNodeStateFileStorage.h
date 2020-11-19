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

#pragma once
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/types.h"

namespace catapult {
	namespace cache {
		class CacheStorage;
		class CatapultCache;
		class CatapultCacheDelta;
		struct SupplementalData;
	}
	namespace config { struct NodeConfiguration; }
	namespace extensions { struct LocalNodeStateRef; }
	namespace model { class ChainScore; }
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace extensions {

	/// Information about state heights.
	struct StateHeights {
		/// Cache height.
		Height Cache;

		/// Storage height.
		Height Storage;
	};

	/// Returns \c true if serialized state is present in \a directory.
	bool HasSerializedState(const config::CatapultDirectory& directory);

	/// Loads dependent state from \a directory and updates \a cache.
	void LoadDependentStateFromDirectory(const config::CatapultDirectory& directory, cache::CatapultCache& cache);

	/// Loads catapult state into \a stateRef from \a directory given \a pluginManager.
	StateHeights LoadStateFromDirectory(
			const config::CatapultDirectory& directory,
			const LocalNodeStateRef& stateRef,
			const plugins::PluginManager& pluginManager);

	/// Serializes local node state.
	class LocalNodeStateSerializer {
	public:
		/// Creates a serializer around specified \a directory.
		explicit LocalNodeStateSerializer(const config::CatapultDirectory& directory);

	public:
		/// Saves state composed of \a cache and \a score.
		void save(const cache::CatapultCache& cache, const model::ChainScore& score) const;

		/// Saves state composed of \a cacheDelta, \a score and \a height using \a cacheStorages.
		void save(
				const cache::CatapultCacheDelta& cacheDelta,
				const std::vector<std::unique_ptr<const cache::CacheStorage>>& cacheStorages,
				const model::ChainScore& score,
				Height height) const;

		/// Moves serialized state to \a destinationDirectory.
		void moveTo(const config::CatapultDirectory& destinationDirectory);

	private:
		config::CatapultDirectory m_directory;
	};

	/// Serializes state composed of \a cache and \a score with checkpointing to \a dataDirectory given \a nodeConfig.
	void SaveStateToDirectoryWithCheckpointing(
			const config::CatapultDataDirectory& dataDirectory,
			const config::NodeConfiguration& nodeConfig,
			const cache::CatapultCache& cache,
			const model::ChainScore& score);
}}
