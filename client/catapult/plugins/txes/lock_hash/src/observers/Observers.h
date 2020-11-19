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
#include "src/model/HashLockNotifications.h"
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	/// Observes changes triggered by hash lock notifications and:
	/// - adds/removes hash lock info to/from hash lock info cache
	DECLARE_OBSERVER(HashLock, model::HashLockNotification)();

	/// Observes hashes of completed, bonded aggregate transactions and:
	/// - credits/debits lock owner
	/// - marks proper hash lock as used/unused
	DECLARE_OBSERVER(CompletedAggregate, model::TransactionNotification)();

	/// Observes block notifications and:
	/// - handles expired hash lock infos
	/// - credits the block signer the mosaics given in the lock info
	DECLARE_OBSERVER(ExpiredHashLockInfo, model::BlockNotification)();
}}
