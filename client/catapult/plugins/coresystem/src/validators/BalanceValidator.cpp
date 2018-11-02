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
#include "catapult/model/Address.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using BalanceTransferNotification = model::BalanceTransferNotification;
	using BalanceDebitNotification = model::BalanceDebitNotification;

	namespace {
		bool FindAccountBalance(const cache::ReadOnlyAccountStateCache& cache, const Key& publicKey, MosaicId mosaicId, Amount& amount) {
			auto accountStateKeyIter = cache.find(publicKey);
			if (accountStateKeyIter.tryGet()) {
				amount = accountStateKeyIter.get().Balances.get(mosaicId);
				return true;
			}

			// if state could not be accessed by public key, try searching by address
			auto accountStateAddressIter = cache.find(model::PublicKeyToAddress(publicKey, cache.networkIdentifier()));
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
			return FindAccountBalance(cache, notification.Sender, notification.MosaicId, amount) && amount >= notification.Amount
					? ValidationResult::Success
					: Failure_Core_Insufficient_Balance;
		}
	}

	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(BalanceTransfer, BalanceTransferNotification, CheckBalance<BalanceTransferNotification>)
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(BalanceDebit, BalanceDebitNotification, CheckBalance<BalanceDebitNotification>)
}}
