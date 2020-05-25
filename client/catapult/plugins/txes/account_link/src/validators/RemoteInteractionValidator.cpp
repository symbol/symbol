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
#include "src/model/AccountKeyLinkTransaction.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/state/AccountState.h"
#include "catapult/validators/ValidatorContext.h"
#include <algorithm>

namespace catapult { namespace validators {

	using Notification = model::AddressInteractionNotification;

	namespace {
		Address GetResolvedKey(const UnresolvedAddress& address, const model::ResolverContext& resolvers) {
			return resolvers.resolve(address);
		}

		bool IsRemote(const cache::ReadOnlyAccountStateCache& cache, const Address& address) {
			auto accountStateIter = cache.find(address);
			return accountStateIter.tryGet() && state::IsRemote(accountStateIter.get().AccountType);
		}
	}

	DEFINE_STATEFUL_VALIDATOR(RemoteInteraction, ([](const Notification& notification, const ValidatorContext& context) {
		if (model::AccountKeyLinkTransaction::Entity_Type == notification.TransactionType)
			return ValidationResult::Success;

		const auto& cache = context.Cache.sub<cache::AccountStateCache>();
		const auto& addresses = notification.ParticipantsByAddress;
		auto predicate = [&cache, &context](const auto& address) {
			return IsRemote(cache, GetResolvedKey(address, context.Resolvers));
		};
		return std::any_of(addresses.cbegin(), addresses.cend(), predicate)
				? Failure_AccountLink_Remote_Account_Participant_Prohibited
				: ValidationResult::Success;
	}));
}}
