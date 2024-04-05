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
#include "src/model/SecretLockNotifications.h"
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	/// Observes changes triggered by secret lock notifications and:
	/// - adds/removes secret lock info to/from secret lock info cache
	/// - debits/credits lock owner
	DECLARE_OBSERVER(SecretLock, model::SecretLockNotification)();

	/// Observes changes triggered by proof notifications and:
	/// - credits/debits proof publisher
	/// - marks proper secret lock as used/unused
	DECLARE_OBSERVER(Proof, model::ProofPublicationNotification)();

	/// Observes block notifications and:
	/// - handles expired secret lock infos
	/// - credits the lock creator the mosaics given in the lock info
	/// - observation is skipped at heights specified by \a skipHeights
	/// - observation is forced at heights specified by \a forceHeights
	DECLARE_OBSERVER(ExpiredSecretLockInfo, model::BlockNotification)(
			const std::unordered_set<Height, utils::BaseValueHasher<Height>>& skipHeights,
			const std::unordered_set<Height, utils::BaseValueHasher<Height>>& forceHeights);
}}
