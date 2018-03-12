#include "LocalNodeTestState.h"
#include "LocalTestUtils.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/state/CatapultState.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBasedStorage.h"

namespace catapult { namespace test {

	struct LocalNodeTestState::Impl {
	public:
		explicit Impl(config::LocalNodeConfiguration&& config, cache::CatapultCache&& cache)
				: m_config(std::move(config))
				, m_cache(std::move(cache))
				, m_storage(std::make_unique<mocks::MockMemoryBasedStorage>())
		{}

	public:
		extensions::LocalNodeStateRef ref() {
			return extensions::LocalNodeStateRef(m_config, m_state, m_cache, m_storage, m_score);
		}

		extensions::LocalNodeStateConstRef cref() const {
			return extensions::LocalNodeStateConstRef(m_config, m_state, m_cache, m_storage, m_score);
		}

	private:
		config::LocalNodeConfiguration m_config;
		state::CatapultState m_state;
		cache::CatapultCache m_cache;
		io::BlockStorageCache m_storage;
		extensions::LocalNodeChainScore m_score;
	};

	LocalNodeTestState::LocalNodeTestState() : LocalNodeTestState(CreateEmptyCatapultCache())
	{}

	LocalNodeTestState::LocalNodeTestState(const model::BlockChainConfiguration& config)
			: LocalNodeTestState(config, "", CreateEmptyCatapultCache(config))
	{}

	LocalNodeTestState::LocalNodeTestState(cache::CatapultCache&& cache)
			: m_pImpl(std::make_unique<Impl>(CreatePrototypicalLocalNodeConfiguration(), std::move(cache)))
	{}

	LocalNodeTestState::LocalNodeTestState(
			const model::BlockChainConfiguration& config,
			const std::string& userDataDirectory,
			cache::CatapultCache&& cache)
			: m_pImpl(std::make_unique<Impl>(
					LoadLocalNodeConfiguration(model::BlockChainConfiguration(config), userDataDirectory),
					std::move(cache)))
	{}

	LocalNodeTestState::~LocalNodeTestState() = default;

	extensions::LocalNodeStateRef LocalNodeTestState::ref() {
		return m_pImpl->ref();
	}

	extensions::LocalNodeStateConstRef LocalNodeTestState::cref() const {
		return m_pImpl->cref();
	}
}}
