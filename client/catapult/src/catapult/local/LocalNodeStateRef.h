#pragma once

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace config { class LocalNodeConfiguration; }
	namespace io { class BlockStorageCache; }
	namespace local { class LocalNodeChainScore; }
	namespace state { struct CatapultState; }
}

namespace catapult { namespace local {

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
		/// The configuration.
		const config::LocalNodeConfiguration& Config;

		/// The state.
		state::CatapultState& State;

		/// The cache.
		cache::CatapultCache& Cache;

		/// The storage.
		io::BlockStorageCache& Storage;

		/// The score.
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
		/// The configuration.
		const config::LocalNodeConfiguration& Config;

		/// The state.
		const state::CatapultState& State;

		/// The cache.
		const cache::CatapultCache& Cache;

		/// The storage.
		const io::BlockStorageCache& Storage;

		/// The score.
		const LocalNodeChainScore& Score;
	};
}}
