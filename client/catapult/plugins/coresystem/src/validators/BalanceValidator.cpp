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

#include "Validators.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/state/CatapultState.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using BalanceTransferNotification = model::BalanceTransferNotification;
	using BalanceDebitNotification = model::BalanceDebitNotification;

	namespace {
		bool TryGetEffectiveAmount(
				const BalanceTransferNotification& notification,
				BlockFeeMultiplier feeMultiplier,
				Amount& effectiveAmount) {
			effectiveAmount = notification.Amount;
			if (BalanceTransferNotification::AmountType::Static == notification.TransferAmountType)
				return true;

			effectiveAmount = Amount(notification.Amount.unwrap() * feeMultiplier.unwrap());
			return std::numeric_limits<Amount::ValueType>::max() / feeMultiplier.unwrap() >= notification.Amount.unwrap();
		}

		bool TryGetEffectiveAmount(const BalanceDebitNotification& notification, BlockFeeMultiplier, Amount& effectiveAmount) {
			effectiveAmount = notification.Amount;
			return true;
		}

		bool FindAccountBalance(const cache::ReadOnlyAccountStateCache& cache, const Address& address, MosaicId mosaicId, Amount& amount) {
			auto accountStateAddressIter = cache.find(address);
			if (accountStateAddressIter.tryGet()) {
				amount = accountStateAddressIter.get().Balances.get(mosaicId);
				return true;
			}

			return false;
		}

		template<typename TNotification>
		ValidationResult CheckBalance(const TNotification& notification, const ValidatorContext& context) {
			const auto& cache = context.Cache.sub<cache::AccountStateCache>();

			Amount amount;
			auto mosaicId = context.Resolvers.resolve(notification.MosaicId);
			if (FindAccountBalance(cache, notification.Sender, mosaicId, amount)) {
				Amount effectiveAmount;
				auto dynamicFeeMultiplier = context.Cache.dependentState().DynamicFeeMultiplier;
				if (TryGetEffectiveAmount(notification, dynamicFeeMultiplier, effectiveAmount) && amount >= effectiveAmount)
					return ValidationResult::Success;
			}

			return Failure_Core_Insufficient_Balance;
		}
	}

	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(BalanceTransfer, BalanceTransferNotification, CheckBalance<BalanceTransferNotification>)
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(BalanceDebit, BalanceDebitNotification, CheckBalance<BalanceDebitNotification>)
}}
