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

#include "Observers.h"
#include "src/cache/MultisigCache.h"

namespace catapult { namespace observers {

	using Notification = model::ModifyMultisigSettingsNotification;

	namespace {
		constexpr uint8_t AddDelta(uint8_t value, int8_t delta) {
			return value + static_cast<uint8_t>(delta);
		}
	}

	DEFINE_OBSERVER(ModifyMultisigSettings, Notification, [](const Notification& notification, const ObserverContext& context) {
		auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
		if (!multisigCache.contains(notification.Signer))
			return;

		auto& multisigEntry = multisigCache.get(notification.Signer);

		int8_t direction = NotifyMode::Commit == context.Mode ? 1 : -1;
		multisigEntry.setMinApproval(AddDelta(multisigEntry.minApproval(), direction * notification.MinApprovalDelta));
		multisigEntry.setMinRemoval(AddDelta(multisigEntry.minRemoval(), direction * notification.MinRemovalDelta));
	});
}}
