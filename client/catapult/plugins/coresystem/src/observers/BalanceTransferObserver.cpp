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

	DEFINE_OBSERVER(BalanceTransfer, model::BalanceTransferNotification, [](
			const model::BalanceTransferNotification& notification,
			const ObserverContext& context) {
		auto& cache = context.Cache.sub<cache::AccountStateCache>();
		auto senderIter = cache.find(notification.Sender);
		auto recipientIter = cache.find(context.Resolvers.resolve(notification.Recipient));

		auto& senderState = senderIter.get();
		auto& recipientState = recipientIter.get();

		auto effectiveAmount = notification.Amount;
		if (model::BalanceTransferNotification::AmountType::Dynamic == notification.TransferAmountType)
			effectiveAmount = Amount(notification.Amount.unwrap() * context.Cache.dependentState().DynamicFeeMultiplier.unwrap());

		auto mosaicId = context.Resolvers.resolve(notification.MosaicId);
		if (NotifyMode::Commit == context.Mode)
			Transfer(senderState, recipientState, mosaicId, effectiveAmount);
		else
			Transfer(recipientState, senderState, mosaicId, effectiveAmount);
	})
}}
