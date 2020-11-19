/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "src/model/AliasNotifications.h"
#include "src/model/NamespaceNotifications.h"
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	// region alias

	/// Observes changes triggered by aliased address notifications and:
	/// - links/unlinks address to namespace
	DECLARE_OBSERVER(AliasedAddress, model::AliasedAddressNotification)();

	/// Observes changes triggered by aliased mosaic id notifications and:
	/// - links/unlinks mosaic id to namespace
	DECLARE_OBSERVER(AliasedMosaicId, model::AliasedMosaicIdNotification)();

	// endregion

	// region namespace

	/// Observes changes triggered by root namespace notifications and:
	/// - creates (root) namespaces
	DECLARE_OBSERVER(RootNamespace, model::RootNamespaceNotification)();

	/// Observes changes triggered by child namespace notifications and:
	/// - creates (child) namespaces
	DECLARE_OBSERVER(ChildNamespace, model::ChildNamespaceNotification)();

	// endregion
}}
