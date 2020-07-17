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

#include "NemesisFundingObserver.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace extensions {

	using Notification = model::BalanceTransferNotification;

	DECLARE_OBSERVER(NemesisFunding, Notification)(const Address& nemesisAddress, NemesisFundingState& fundingState) {
		return MAKE_OBSERVER(NemesisFunding, Notification, ([&nemesisAddress, &fundingState](
				const Notification& notification,
				const observers::ObserverContext& context) {
			// since this is only used by NemesisBlockLoader, it only needs to support commit because nemesis can't be rolled back
			if (observers::NotifyMode::Commit != context.Mode || Height(1) != context.Height)
				CATAPULT_THROW_INVALID_ARGUMENT("NemesisFundingObserver only supports commit mode for nemesis block");

			// never fund non-nemesis accounts
			if (nemesisAddress != notification.Sender)
				return;

			auto& cache = context.Cache.sub<cache::AccountStateCache>();
			cache.addAccount(notification.Sender, context.Height);

			auto senderIter = cache.find(notification.Sender);
			auto& senderState = senderIter.get();

			auto mosaicId = context.Resolvers.resolve(notification.MosaicId);
			fundingState.TotalFundedMosaics.credit(mosaicId, notification.Amount);

			// if the account state balance is zero when the first transfer is made, implicitly fund the nemesis account
			// assume that all mosaics are funded in same way (i.e. nemesis cannot have mix of implicitly and explicitly funded mosaics)
			if (NemesisFundingType::Unknown == fundingState.FundingType) {
				fundingState.FundingType = Amount() != senderState.Balances.get(mosaicId)
						? NemesisFundingType::Explicit
						: NemesisFundingType::Implicit;
			}

			if (NemesisFundingType::Explicit == fundingState.FundingType)
				return;

			senderState.Balances.credit(mosaicId, notification.Amount);
		}));
	}
}}
