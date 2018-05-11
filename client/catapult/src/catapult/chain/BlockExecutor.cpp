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

#include "BlockExecutor.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Block.h"
#include "catapult/observers/EntityObserver.h"

namespace catapult { namespace chain {

	namespace {
		constexpr observers::ObserverContext CreateObserverContext(
				const observers::ObserverState& state,
				const model::Block& block,
				observers::NotifyMode mode) {
			return observers::ObserverContext(state, block.Height, mode);
		}

		void ObserveAll(
				const observers::EntityObserver& observer,
				const observers::ObserverContext& context,
				const model::WeakEntityInfos& entityInfos) {
			for (const auto& entityInfo : entityInfos)
				observer.notify(entityInfo, context);
		}
	}

	void ExecuteBlock(
			const model::BlockElement& blockElement,
			const observers::EntityObserver& observer,
			const observers::ObserverState& state) {
		model::WeakEntityInfos entityInfos;
		model::ExtractEntityInfos(blockElement, entityInfos);

		auto context = CreateObserverContext(state, blockElement.Block, observers::NotifyMode::Commit);
		ObserveAll(observer, context, entityInfos);
	}

	void RollbackBlock(
			const model::BlockElement& blockElement,
			const observers::EntityObserver& observer,
			const observers::ObserverState& state) {
		model::WeakEntityInfos entityInfos;
		model::ExtractEntityInfos(blockElement, entityInfos);
		std::reverse(entityInfos.begin(), entityInfos.end());

		auto context = CreateObserverContext(state, blockElement.Block, observers::NotifyMode::Rollback);
		ObserveAll(observer, context, entityInfos);

		// commit removals
		state.Cache.sub<cache::AccountStateCache>().commitRemovals();
	}
}}
