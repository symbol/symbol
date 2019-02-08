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
#include "catapult/model/Mosaic.h"

namespace catapult { namespace observers {

	namespace {
		void ApplyFee(
				state::AccountState& accountState,
				NotifyMode notifyMode,
				const model::Mosaic& feeMosaic,
				ObserverStatementBuilder& statementBuilder) {
			if (NotifyMode::Rollback == notifyMode) {
				accountState.Balances.debit(feeMosaic.MosaicId, feeMosaic.Amount);
				return;
			}

			accountState.Balances.credit(feeMosaic.MosaicId, feeMosaic.Amount);

			// add harvest fee receipt
			auto receiptType = model::Receipt_Type_Harvest_Fee;
			model::BalanceChangeReceipt receipt(receiptType, accountState.PublicKey, feeMosaic.MosaicId, feeMosaic.Amount);
			statementBuilder.addReceipt(receipt);
		}
	}

	DECLARE_OBSERVER(HarvestFee, model::BlockNotification)(MosaicId currencyMosaicId) {
		return MAKE_OBSERVER(HarvestFee, model::BlockNotification, ([currencyMosaicId](const auto& notification, auto& context) {
			// credit the harvester
			auto& cache = context.Cache.template sub<cache::AccountStateCache>();
			auto accountStateIter = cache.find(notification.Signer);
			auto& harvesterAccountState = accountStateIter.get();

			model::Mosaic feeMosaic{ currencyMosaicId, notification.TotalFee };
			if (state::AccountType::Remote != harvesterAccountState.AccountType) {
				ApplyFee(harvesterAccountState, context.Mode, feeMosaic, context.StatementBuilder());
				return;
			}

			auto linkedAccountStateIter = cache.find(harvesterAccountState.LinkedAccountKey);
			auto& linkedAccountState = linkedAccountStateIter.get();

			// this check is merely a precaution and will only fire if there is a bug that has corrupted links
			RequireLinkedRemoteAndMainAccounts(harvesterAccountState, linkedAccountState);

			ApplyFee(linkedAccountState, context.Mode, feeMosaic, context.StatementBuilder());
		}));
	}
}}
