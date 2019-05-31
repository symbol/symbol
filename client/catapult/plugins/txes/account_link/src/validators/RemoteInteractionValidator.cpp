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
#include "src/model/AccountLinkTransaction.h"
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

		const Key& GetResolvedKey(const Key& key, const model::ResolverContext&) {
			return key;
		}

		template<typename TKey>
		bool IsRemote(const cache::ReadOnlyAccountStateCache& cache, const TKey& key) {
			auto accountStateIter = cache.find(key);
			return accountStateIter.tryGet() && state::IsRemote(accountStateIter.get().AccountType);
		}
	}

	DEFINE_STATEFUL_VALIDATOR(RemoteInteraction, ([](const auto& notification, const auto& context) {
		if (model::AccountLinkTransaction::Entity_Type == notification.TransactionType)
			return ValidationResult::Success;

		const auto& cache = context.Cache.template sub<cache::AccountStateCache>();
		const auto& addresses = notification.ParticipantsByAddress;
		const auto& keys = notification.ParticipantsByKey;
		auto predicate = [&cache, &context](const auto& key) {
			return IsRemote(cache, GetResolvedKey(key, context.Resolvers));
		};
		return std::any_of(addresses.cbegin(), addresses.cend(), predicate) || std::any_of(keys.cbegin(), keys.cend(), predicate)
				? Failure_AccountLink_Remote_Account_Participant_Not_Allowed
				: ValidationResult::Success;
	}));
}}
