#include "Observers.h"
#include "src/cache/MosaicCache.h"

namespace catapult { namespace observers {

	// note that this is a block notification observer to ensure that pruning runs every block
	NotificationObserverPointerT<model::BlockNotification> CreateMosaicPruningObserver(uint32_t maxRollbackBlocks) {
		return std::make_unique<FunctionalNotificationObserverT<model::BlockNotification>>(
				"MosaicPruningObserver",
				[maxRollbackBlocks](const auto&, const ObserverContext& context) {
				if (NotifyMode::Rollback == context.Mode)
					return;

				if (maxRollbackBlocks >= context.Height.unwrap())
					return;

				auto pruneHeight = Height(context.Height.unwrap() - maxRollbackBlocks);
				auto& mosaicCache = context.Cache.sub<cache::MosaicCache>();
				mosaicCache.prune(pruneHeight);
		});
	}
}}
