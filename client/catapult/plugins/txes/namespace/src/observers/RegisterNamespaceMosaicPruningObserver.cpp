#include "Observers.h"
#include "src/cache/MosaicCache.h"
#include "src/cache/NamespaceCache.h"
#include "src/model/NamespaceLifetimeConstraints.h"

namespace catapult { namespace observers {

	using Notification = model::RootNamespaceNotification;

	namespace {
		bool IsOwnerExtendingRootLifetime(
				const Key& signer,
				const state::RootNamespace& root,
				Height height,
				const model::NamespaceLifetimeConstraints& constraints) {
			return root.owner() == signer && constraints.IsWithinLifetimePlusDuration(root.lifetime().End, height);
		}
	}

	DECLARE_OBSERVER(RegisterNamespaceMosaicPruning, Notification)(const model::NamespaceLifetimeConstraints& constraints) {
		return MAKE_OBSERVER(RegisterNamespaceMosaicPruning, Notification, [constraints](
				const auto& notification,
				const ObserverContext& context) {
			if (NotifyMode::Rollback == context.Mode)
				return;

			// note that the pruning observer should run before the transaction itself is observed
			const auto& namespaceCache = context.Cache.sub<cache::NamespaceCache>();
			if (!namespaceCache.contains(notification.NamespaceId))
				return;

			const auto& root = namespaceCache.get(notification.NamespaceId).root();
			if (IsOwnerExtendingRootLifetime(notification.Signer, root, context.Height, constraints))
				return;

			auto& mosaicCache = context.Cache.sub<cache::MosaicCache>();
			mosaicCache.remove(root.id());
			for (const auto& pair : root.children())
				mosaicCache.remove(pair.first);
		});
	}
}}
