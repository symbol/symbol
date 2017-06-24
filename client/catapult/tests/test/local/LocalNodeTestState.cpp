#include "LocalNodeTestState.h"
#include "LocalTestUtils.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/local/LocalNodeChainScore.h"
#include "catapult/state/CatapultState.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"

namespace catapult { namespace test {

	struct LocalNodeTestState::Impl {
	public:
		explicit Impl(config::LocalNodeConfiguration&& config, cache::CatapultCache&& cache)
				: m_config(std::move(config))
				, m_cache(std::move(cache))
				, m_storage(std::make_unique<mocks::MemoryBasedStorage>())
		{}

	public:
		local::LocalNodeStateRef ref() {
			return local::LocalNodeStateRef(m_config, m_state, m_cache, m_storage, m_score);
		}

		local::LocalNodeStateConstRef cref() const {
			return local::LocalNodeStateConstRef(m_config, m_state, m_cache, m_storage, m_score);
		}

	private:
		config::LocalNodeConfiguration m_config;
		state::CatapultState m_state;
		cache::CatapultCache m_cache;
		io::BlockStorageCache m_storage;
		local::LocalNodeChainScore m_score;
	};

	LocalNodeTestState::LocalNodeTestState() : LocalNodeTestState(CreateEmptyCatapultCache())
	{}

	LocalNodeTestState::LocalNodeTestState(const model::BlockChainConfiguration& config)
			: m_pImpl(std::make_unique<Impl>(
					LoadLocalNodeConfiguration(model::BlockChainConfiguration(config), local_node_flags::None, ""),
					CreateEmptyCatapultCache(config)))
	{}

	LocalNodeTestState::LocalNodeTestState(cache::CatapultCache&& cache)
			: m_pImpl(std::make_unique<Impl>(CreatePrototypicalLocalNodeConfiguration(), std::move(cache)))
	{}

	LocalNodeTestState::~LocalNodeTestState() = default;

	local::LocalNodeStateRef LocalNodeTestState::ref() {
		return m_pImpl->ref();
	}

	local::LocalNodeStateConstRef LocalNodeTestState::cref() const {
		return m_pImpl->cref();
	}
}}
