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
#include "catapult/cache_core/AccountStateCacheUtils.h"
#include "catapult/model/InflationCalculator.h"
#include "catapult/model/Mosaic.h"

namespace catapult { namespace observers {

	namespace {
		using Notification = model::BlockNotification;

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

			// add fee receipt
			auto receiptType = model::Receipt_Type_Harvest_Fee;
			model::BalanceChangeReceipt receipt(receiptType, accountState.PublicKey, feeMosaic.MosaicId, feeMosaic.Amount);
			statementBuilder.addReceipt(receipt);
		}

		void ApplyFee(const Key& publicKey, const model::Mosaic& feeMosaic, ObserverContext& context) {
			auto& cache = context.Cache.sub<cache::AccountStateCache>();
			cache::ProcessForwardedAccountState(cache, publicKey, [&feeMosaic, &context](auto& accountState) {
				ApplyFee(accountState, context.Mode, feeMosaic, context.StatementBuilder());
			});
		}

		bool ShouldShareFees(const Key& signer, const Key& harvesterBeneficiary, uint8_t harvestBeneficiaryPercentage) {
			return 0u < harvestBeneficiaryPercentage && signer != harvesterBeneficiary;
		}
	}

	DECLARE_OBSERVER(HarvestFee, Notification)(
			MosaicId currencyMosaicId,
			uint8_t harvestBeneficiaryPercentage,
			const model::InflationCalculator& calculator) {
		return MAKE_OBSERVER(HarvestFee, Notification, ([currencyMosaicId, harvestBeneficiaryPercentage, calculator](
				const Notification& notification,
				ObserverContext& context) {
			auto inflationAmount = calculator.getSpotAmount(context.Height);
			auto totalAmount = notification.TotalFee + inflationAmount;
			auto beneficiaryAmount = ShouldShareFees(notification.Signer, notification.Beneficiary, harvestBeneficiaryPercentage)
					? Amount(totalAmount.unwrap() * harvestBeneficiaryPercentage / 100)
					: Amount();
			auto harvesterAmount = totalAmount - beneficiaryAmount;

			// always create receipt for harvester
			ApplyFee(notification.Signer, { currencyMosaicId, harvesterAmount }, context);

			// only if amount is non-zero create receipt for beneficiary account
			if (Amount() != beneficiaryAmount)
				ApplyFee(notification.Beneficiary, { currencyMosaicId, beneficiaryAmount }, context);

			// add inflation receipt
			if (Amount() != inflationAmount && NotifyMode::Commit == context.Mode) {
				model::InflationReceipt receipt(model::Receipt_Type_Inflation, currencyMosaicId, inflationAmount);
				context.StatementBuilder().addReceipt(receipt);
			}
		}));
	}
}}
