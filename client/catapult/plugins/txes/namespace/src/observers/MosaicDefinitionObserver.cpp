#include "Observers.h"
#include "src/cache/MosaicCache.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	namespace {
		void ZeroBalance(state::AccountState& accountState, MosaicId mosaicId) {
			auto ownerBalance = accountState.Balances.get(mosaicId);
			accountState.Balances.debit(mosaicId, ownerBalance);
		}
	}

	DEFINE_OBSERVER(MosaicDefinition, model::MosaicDefinitionNotification, [](const auto& notification, const ObserverContext& context) {
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto& cache = context.Cache.sub<cache::MosaicCache>();

		// always zero the owner's balance when a mosaic definition changes (in case of rollback, it will be fixed below)
		auto& ownerState = accountStateCache.get(notification.Signer);
		ZeroBalance(ownerState, notification.MosaicId);

		if (NotifyMode::Rollback == context.Mode) {
			cache.remove(notification.MosaicId);

			// mosaic is not completely removed from the cache if it initially had a history depth greater than one
			if (cache.contains(notification.MosaicId)) {
				// set the owner's balance to the full supply
				const auto& mosaicEntry = cache.get(notification.MosaicId);
				ownerState.Balances.credit(notification.MosaicId, mosaicEntry.supply());
			}

			return;
		}

		auto definition = state::MosaicDefinition(context.Height, notification.Signer, notification.Properties);
		cache.insert(state::MosaicEntry(notification.ParentId, notification.MosaicId, definition));
	});
}}
