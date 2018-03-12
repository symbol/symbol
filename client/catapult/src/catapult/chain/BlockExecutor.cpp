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
