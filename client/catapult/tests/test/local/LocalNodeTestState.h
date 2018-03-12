#pragma once
#include "catapult/extensions/LocalNodeStateRef.h"
#include <memory>
#include <string>

namespace catapult { namespace model { struct BlockChainConfiguration; } }

namespace catapult { namespace test {

	/// Local node test state.
	class LocalNodeTestState {
	public:
		/// Creates default state.
		LocalNodeTestState();

		/// Creates default state around \a config.
		explicit LocalNodeTestState(const model::BlockChainConfiguration& config);

		/// Creates default state around \a cache.
		explicit LocalNodeTestState(cache::CatapultCache&& cache);

		/// Creates default state around \a config, \a userDataDirectory and \a cache.
		LocalNodeTestState(
				const model::BlockChainConfiguration& config,
				const std::string& userDataDirectory,
				cache::CatapultCache&& cache);

		/// Destroys the state.
		~LocalNodeTestState();

	public:
		/// Returns a state ref.
		extensions::LocalNodeStateRef ref();

		/// Returns a const state ref.
		extensions::LocalNodeStateConstRef cref() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}}
