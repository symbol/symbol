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
#include "ExecutionConfiguration.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"

namespace catapult {
	namespace cache {
		class CatapultCacheDelta;
		class CatapultCacheView;
	}
	namespace model {
		struct BlockChainConfiguration;
		class BlockStatementBuilder;
	}
	namespace observers { struct ObserverState; }
}

namespace catapult { namespace chain {

	/// Builder for creating process (observer and validator) contexts.
	class ProcessContextsBuilder {
	public:
		/// Creates a builder around \a height, \a blockTime and \a executionContextConfig.
		ProcessContextsBuilder(Height height, Timestamp blockTime, const ExecutionContextConfiguration& executionContextConfig);

	public:
		/// Sets a catapult cache \a view.
		void setCache(const cache::CatapultCacheView& view);

		/// Sets a catapult cache \a delta.
		void setCache(cache::CatapultCacheDelta& delta);

		/// Sets a block statement builder (\a blockStatementBuilder) to use.
		void setBlockStatementBuilder(model::BlockStatementBuilder& blockStatementBuilder);

		/// Sets a catapult observer \a state.
		void setObserverState(const observers::ObserverState& state);

	public:
		/// Builds an observer context.
		observers::ObserverContext buildObserverContext();

		/// Builds a validator context.
		validators::ValidatorContext buildValidatorContext();

	private:
		model::NotificationContext buildNotificationContext();

	private:
		Height m_height;
		Timestamp m_blockTime;
		ExecutionContextConfiguration m_executionContextConfig;

		const cache::CatapultCacheView* m_pCacheView;
		cache::CatapultCacheDelta* m_pCacheDelta;
		std::unique_ptr<cache::ReadOnlyCatapultCache> m_pReadOnlyCache;

		model::BlockStatementBuilder* m_pBlockStatementBuilder;
	};
}}
