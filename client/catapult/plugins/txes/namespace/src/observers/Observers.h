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
	DECLARE_OBSERVER(MosaicDefinition, model::MosaicDefinitionNotification)();

	/// Observes changes triggered by mosaic supply change notifications, including:
	/// - increase or decrease of supply
	DECLARE_OBSERVER(MosaicSupplyChange, model::MosaicSupplyChangeNotification)();

	// endregion

	// region namespace

	/// Observes changes triggered by root namespace notifications, including:
	/// - creation of namespaces
	DECLARE_OBSERVER(RootNamespace, model::RootNamespaceNotification)();

	/// Observes changes triggered by child namespace notifications, including:
	/// - creation of namespaces
	DECLARE_OBSERVER(ChildNamespace, model::ChildNamespaceNotification)();

	/// Observes changes triggered by root namespace notifications, including:
	/// - pruning triggered by root namespace replacement given \a constraints
	DECLARE_OBSERVER(RegisterNamespaceMosaicPruning, model::RootNamespaceNotification)(
			const model::NamespaceLifetimeConstraints& constraints);

	// endregion
}}
