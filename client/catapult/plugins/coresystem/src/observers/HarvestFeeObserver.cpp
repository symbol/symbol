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
			auto& cache = context.Cache.template sub<cache::AccountStateCache>();
			auto accountStateIter = cache.find(publicKey);
			auto& accountState = accountStateIter.get();

			if (state::AccountType::Remote != accountState.AccountType) {
				ApplyFee(accountState, context.Mode, feeMosaic, context.StatementBuilder());
				return;
			}

			auto linkedAccountStateIter = cache.find(accountState.LinkedAccountKey);
			auto& linkedAccountState = linkedAccountStateIter.get();

			// this check is merely a precaution and will only fire if there is a bug that has corrupted links
			RequireLinkedRemoteAndMainAccounts(accountState, linkedAccountState);

			ApplyFee(linkedAccountState, context.Mode, feeMosaic, context.StatementBuilder());
		}

		bool ShouldShareFees(const Key& signer, const Key& harvesterBeneficiary, uint8_t harvestBeneficiaryPercentage) {
			return 0u < harvestBeneficiaryPercentage && Key() != harvesterBeneficiary && signer != harvesterBeneficiary;
		}
	}

	DECLARE_OBSERVER(HarvestFee, Notification)(
			MosaicId currencyMosaicId,
			uint8_t harvestBeneficiaryPercentage,
			const model::InflationCalculator& calculator) {
		auto mosaicId = currencyMosaicId;
		auto percentage = harvestBeneficiaryPercentage;
		return MAKE_OBSERVER(HarvestFee, Notification, ([mosaicId, percentage, calculator](const auto& notification, auto& context) {
			auto inflationAmount = calculator.getSpotAmount(context.Height);
			auto totalAmount = notification.TotalFee + inflationAmount;
			auto beneficiaryAmount = ShouldShareFees(notification.Signer, notification.Beneficiary, percentage)
					? Amount(totalAmount.unwrap() * percentage / 100)
					: Amount();
			auto harvesterAmount = totalAmount - beneficiaryAmount;

			// always create receipt for harvester
			ApplyFee(notification.Signer, { mosaicId, harvesterAmount }, context);

			// only if amount is non-zero create receipt for beneficiary account
			if (Amount() != beneficiaryAmount)
				ApplyFee(notification.Beneficiary, { mosaicId, beneficiaryAmount }, context);

			// add inflation receipt
			if (Amount() != inflationAmount && NotifyMode::Commit == context.Mode) {
				model::InflationReceipt receipt(model::Receipt_Type_Inflation, mosaicId, inflationAmount);
				context.StatementBuilder().addReceipt(receipt);
			}
		}));
	}
}}
