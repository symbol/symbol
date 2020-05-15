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
#include "catapult/cache/CatapultCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/observers/ObserverContext.h"
#include "catapult/state/AccountState.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"

namespace catapult { namespace test {

	/// Observer test context that wraps an observer context.
	template<typename TCacheFactory>
	class ObserverTestContextT {
	public:
		/// Creates a test context around \a mode.
		explicit ObserverTestContextT(observers::NotifyMode mode) : ObserverTestContextT(mode, Height(444))
		{}

		/// Creates a test context around \a mode and \a height.
		ObserverTestContextT(observers::NotifyMode mode, Height height)
				: ObserverTestContextT(mode, height, model::BlockChainConfiguration::Uninitialized())
		{}

		/// Creates a test context around \a mode, \a height and \a config.
		ObserverTestContextT(observers::NotifyMode mode, Height height, const model::BlockChainConfiguration& config)
				: m_cache(TCacheFactory::Create(config))
				, m_cacheDelta(m_cache.createDelta())
				, m_context(
						model::NotificationContext(height, CreateResolverContextXor()),
						observers::ObserverState(m_cacheDelta, m_blockStatementBuilder),
						mode)
		{}

	public:
		/// Gets the observer context.
		observers::ObserverContext& observerContext() {
			return m_context;
		}

		/// Gets the catapult cache delta.
		const cache::CatapultCacheDelta& cache() const {
			return m_cacheDelta;
		}

		/// Gets the catapult cache delta.
		cache::CatapultCacheDelta& cache() {
			return m_cacheDelta;
		}

		/// Gets the catapult state.
		const state::CatapultState& state() const {
			return m_cacheDelta.dependentState();
		}

		/// Gets the catapult state.
		state::CatapultState& state() {
			return m_cacheDelta.dependentState();
		}

		/// Gets the block statement builder.
		model::BlockStatementBuilder& statementBuilder() {
			return m_blockStatementBuilder;
		}

	public:
		/// Commits all changes to the underlying cache.
		void commitCacheChanges() {
			m_cache.commit(Height());
		}

	private:
		cache::CatapultCache m_cache;
		cache::CatapultCacheDelta m_cacheDelta;
		model::BlockStatementBuilder m_blockStatementBuilder;

		observers::ObserverContext m_context;
	};

	/// Default observer test context that wraps a cache composed of core caches only.
	class ObserverTestContext : public ObserverTestContextT<CoreSystemCacheFactory> {
		using ObserverTestContextT::ObserverTestContextT;
	};
}}
