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
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	namespace {
		using Notification = model::BlockNotification;

		std::string GetObserverName(NotifyMode mode) {
			std::string name("HighValueAccount");
			name += NotifyMode::Commit == mode ? "Commit" : "Rollback";
			name += "Observer";
			return name;
		}
	}

	DECLARE_OBSERVER(HighValueAccount, Notification)(NotifyMode mode) {
		using ObserverType = observers::FunctionalNotificationObserverT<Notification>;
		return std::make_unique<ObserverType>(GetObserverName(mode), [mode](const Notification&, ObserverContext& context) {
			if (context.Mode != mode)
				return;

			context.Cache.sub<cache::AccountStateCache>().updateHighValueAccounts(context.Height);
		});
	}
}}
