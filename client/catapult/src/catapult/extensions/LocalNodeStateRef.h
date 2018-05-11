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

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace config { class LocalNodeConfiguration; }
	namespace extensions { class LocalNodeChainScore; }
	namespace io { class BlockStorageCache; }
	namespace state { struct CatapultState; }
}

namespace catapult { namespace extensions {

	/// A reference to a local node's basic state.
	struct LocalNodeStateRef {
	public:
		/// Creates a local node state ref referencing state composed of
		/// \a config, \a state, \a cache, \a storage and \a score.
		LocalNodeStateRef(
				const config::LocalNodeConfiguration& config,
				state::CatapultState& state,
				cache::CatapultCache& cache,
				io::BlockStorageCache& storage,
				LocalNodeChainScore& score)
				: Config(config)
				, State(state)
				, Cache(cache)
				, Storage(storage)
				, Score(score)
		{}

	public:
		/// Local node configuration.
		const config::LocalNodeConfiguration& Config;

		/// Local node state.
		state::CatapultState& State;

		/// Local node cache.
		cache::CatapultCache& Cache;

		/// Local node storage.
		io::BlockStorageCache& Storage;

		/// Local node score.
		LocalNodeChainScore& Score;
	};

	/// A const reference to a local node's basic state.
	struct LocalNodeStateConstRef {
	public:
		/// Creates a local node state const ref referencing state composed of
		/// \a config, \a state, \a cache, \a storage and \a score.
		LocalNodeStateConstRef(
				const config::LocalNodeConfiguration& config,
				const state::CatapultState& state,
				const cache::CatapultCache& cache,
				const io::BlockStorageCache& storage,
				const LocalNodeChainScore& score)
				: Config(config)
				, State(state)
				, Cache(cache)
				, Storage(storage)
				, Score(score)
		{}

	public:
		/// Local node configuration.
		const config::LocalNodeConfiguration& Config;

		/// Local node state.
		const state::CatapultState& State;

		/// Local node cache.
		const cache::CatapultCache& Cache;

		/// Local node storage.
		const io::BlockStorageCache& Storage;

		/// Local node score.
		const LocalNodeChainScore& Score;
	};
}}
