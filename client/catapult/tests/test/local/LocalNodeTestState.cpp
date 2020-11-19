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

#include "LocalNodeTestState.h"
#include "LocalTestUtils.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/state/CatapultState.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"

namespace catapult { namespace test {

	struct LocalNodeTestState::Impl {
	public:
		Impl(config::CatapultConfiguration&& config, cache::CatapultCache&& cache)
				: m_config(std::move(config))
				, m_cache(std::move(cache))
				, m_storage(std::make_unique<mocks::MockMemoryBlockStorage>(), std::make_unique<mocks::MockMemoryBlockStorage>())
		{}

	public:
		extensions::LocalNodeStateRef ref() {
			return extensions::LocalNodeStateRef(m_config, m_cache, m_storage, m_score);
		}

	private:
		config::CatapultConfiguration m_config;
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
			: m_pImpl(std::make_unique<Impl>(CreatePrototypicalCatapultConfiguration(), std::move(cache)))
	{}

	LocalNodeTestState::LocalNodeTestState(
			const model::BlockChainConfiguration& config,
			const std::string& userDataDirectory,
			cache::CatapultCache&& cache)
			: m_pImpl(std::make_unique<Impl>(
					CreatePrototypicalCatapultConfiguration(model::BlockChainConfiguration(config), userDataDirectory),
					std::move(cache)))
	{}

	LocalNodeTestState::~LocalNodeTestState() = default;

	extensions::LocalNodeStateRef LocalNodeTestState::ref() {
		return m_pImpl->ref();
	}
}}
