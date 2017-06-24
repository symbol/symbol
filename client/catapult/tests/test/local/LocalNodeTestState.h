#pragma once
#include "catapult/local/LocalNodeStateRef.h"
#include <memory>

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

		/// Destroys the state.
		~LocalNodeTestState();

	public:
		/// Returns a state ref.
		local::LocalNodeStateRef ref();

		/// Returns a const state ref.
		local::LocalNodeStateConstRef cref() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}}
