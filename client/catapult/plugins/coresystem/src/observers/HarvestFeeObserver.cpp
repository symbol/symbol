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

		class FeeApplier {
		public:
			FeeApplier(MosaicId currencyMosaicId, ObserverContext& context)
					: m_currencyMosaicId(currencyMosaicId)
					, m_context(context)
			{}

		public:
			void apply(const Address& address, Amount amount) {
				auto& cache = m_context.Cache.sub<cache::AccountStateCache>();
				auto feeMosaic = model::Mosaic{ m_currencyMosaicId, amount };
				cache::ProcessForwardedAccountState(cache, address, [&feeMosaic, &context = m_context](auto& accountState) {
					ApplyFee(accountState, context.Mode, feeMosaic, context.StatementBuilder());
				});
			}

		private:
			static void ApplyFee(
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
				model::BalanceChangeReceipt receipt(receiptType, accountState.Address, feeMosaic.MosaicId, feeMosaic.Amount);
				statementBuilder.addReceipt(receipt);
			}

		private:
			MosaicId m_currencyMosaicId;
			ObserverContext& m_context;
		};

		bool ShouldShareFees(const Notification& notification, uint8_t harvestBeneficiaryPercentage) {
			return 0u < harvestBeneficiaryPercentage && notification.Harvester != notification.Beneficiary;
		}
	}

	DECLARE_OBSERVER(HarvestFee, Notification)(const HarvestFeeOptions& options, const model::InflationCalculator& calculator) {
		return MAKE_OBSERVER(HarvestFee, Notification, ([options, calculator](const Notification& notification, ObserverContext& context) {
			auto inflationAmount = calculator.getSpotAmount(context.Height);
			auto totalAmount = notification.TotalFee + inflationAmount;

			auto networkAmount = Amount(totalAmount.unwrap() * options.HarvestNetworkPercentage / 100);
			auto beneficiaryAmount = ShouldShareFees(notification, options.HarvestBeneficiaryPercentage)
					? Amount(totalAmount.unwrap() * options.HarvestBeneficiaryPercentage / 100)
					: Amount();
			auto harvesterAmount = totalAmount - networkAmount - beneficiaryAmount;

			// always create receipt for harvester
			FeeApplier applier(options.CurrencyMosaicId, context);
			applier.apply(notification.Harvester, harvesterAmount);

			// only if amount is non-zero create receipt for network sink account
			if (Amount() != networkAmount)
				applier.apply(options.HarvestNetworkFeeSinkAddress, networkAmount);

			// only if amount is non-zero create receipt for beneficiary account
			if (Amount() != beneficiaryAmount)
				applier.apply(notification.Beneficiary, beneficiaryAmount);

			// add inflation receipt
			if (Amount() != inflationAmount && NotifyMode::Commit == context.Mode) {
				model::InflationReceipt receipt(model::Receipt_Type_Inflation, options.CurrencyMosaicId, inflationAmount);
				context.StatementBuilder().addReceipt(receipt);
			}
		}));
	}
}}
