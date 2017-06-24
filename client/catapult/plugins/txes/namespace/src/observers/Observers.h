#pragma once
#include "src/model/MosaicNotifications.h"
#include "src/model/NamespaceNotifications.h"
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace model { struct NamespaceLifetimeConstraints; } }

namespace catapult { namespace observers {

	// region mosaic

	/// Observes changes triggered by mosaic definition notifications, including:
	/// - creation of mosaics
	NotificationObserverPointerT<model::MosaicDefinitionNotification> CreateMosaicDefinitionObserver();

	/// Observes changes triggered by mosaic supply change notifications, including:
	/// - increase or decrease of supply
	NotificationObserverPointerT<model::MosaicSupplyChangeNotification> CreateMosaicSupplyChangeObserver();

	/// Observes balance transfer notifications in the nemesis block and:
	/// - adjusts nemesis supply
	NotificationObserverPointerT<model::BalanceTransferNotification> CreateNemesisBalanceChangeObserver();

	/// Observes block notifications and triggers pruning if necessary.
	/// Pruning is done every block using the specified \a maxRollbackBlocks.
	NotificationObserverPointerT<model::BlockNotification> CreateMosaicPruningObserver(uint32_t maxRollbackBlocks);

	// endregion

	// region namespace

	/// Observes changes triggered by root namespace notifications, including:
	/// - creation of namespaces
	NotificationObserverPointerT<model::RootNamespaceNotification> CreateRootNamespaceObserver();

	/// Observes changes triggered by child namespace notifications, including:
	/// - creation of namespaces
	NotificationObserverPointerT<model::ChildNamespaceNotification> CreateChildNamespaceObserver();

	/// Observes block notifications and triggers pruning if necessary.
	/// Pruning is done every \a pruneInterval blocks using the specified \a gracePeriodDuration.
	NotificationObserverPointerT<model::BlockNotification> CreateNamespacePruningObserver(
			ArtifactDuration gracePeriodDuration,
			size_t pruneInterval);

	/// Observes changes triggered by root namespace notifications, including:
	/// - pruning triggered by root namespace replacement given \a constraints
	NotificationObserverPointerT<model::RootNamespaceNotification> CreateRegisterNamespaceMosaicPruningObserver(
			const model::NamespaceLifetimeConstraints& constraints);

	// endregion
}}
