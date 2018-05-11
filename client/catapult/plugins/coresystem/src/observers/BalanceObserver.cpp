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
		void Transfer(state::AccountState& debitState, state::AccountState& creditState, MosaicId mosaicId, Amount amount) {
			debitState.Balances.debit(mosaicId, amount);
			creditState.Balances.credit(mosaicId, amount);
		}
	}

	DEFINE_OBSERVER(Balance, model::BalanceTransferNotification, [](const auto& notification, const ObserverContext& context) {
		auto& cache = context.Cache.sub<cache::AccountStateCache>();
		auto& senderState = cache.get(notification.Sender);
		auto& recipientState = cache.get(notification.Recipient);

		if (NotifyMode::Commit == context.Mode)
			Transfer(senderState, recipientState, notification.MosaicId, notification.Amount);
		else
			Transfer(recipientState, senderState, notification.MosaicId, notification.Amount);
	});
}}
