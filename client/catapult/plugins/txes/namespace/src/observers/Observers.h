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
