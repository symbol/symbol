/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "HarvestingObservers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"

namespace catapult { namespace harvesting {

	namespace {
		class CollectingAccountVisitor {
		public:
			CollectingAccountVisitor(const observers::ObserverContext& context, HarvestingAffectedAccounts& accounts)
					: m_context(context)
					, m_accounts(accounts)
			{}

		public:
			void visit(const model::ResolvableAddress& address) {
				notify(address.resolved(m_context.Resolvers), m_accounts.Addresses);
			}

			void visit(const Key& publicKey) {
				notify(publicKey, m_accounts.PublicKeys);

				// notify address separately so that account public key can be rolled back independent of addresss
				auto& accountStateCacheDelta = m_context.Cache.template sub<cache::AccountStateCache>();
				auto address = model::PublicKeyToAddress(publicKey, accountStateCacheDelta.networkIdentifier());
				notify(address, m_accounts.Addresses);

				if (observers::NotifyMode::Commit != m_context.Mode)
					accountStateCacheDelta.queueRemove(address, m_context.Height);
			}

		private:
			template<typename TAccountIdentifier>
			void notify(
					const TAccountIdentifier& accountIdentifier,
					RefCountedAccountIdentifiers<TAccountIdentifier>& accountIdentifiers) const {
				auto iter = accountIdentifiers.find(accountIdentifier);
				if (observers::NotifyMode::Commit == m_context.Mode) {
					if (accountIdentifiers.cend() == iter)
						accountIdentifiers.emplace(accountIdentifier, 1);
					else
						++iter->second;

					return;
				}

				// rollback - so account must have been previously added
				if (0 == --iter->second)
					accountIdentifiers.erase(iter);
			}

		private:
			const observers::ObserverContext& m_context;
			HarvestingAffectedAccounts& m_accounts;
		};
	}

	DECLARE_OBSERVER(HarvestingAccountAddress, model::AccountAddressNotification)(HarvestingAffectedAccounts& accounts) {
		return MAKE_OBSERVER(HarvestingAccountAddress, model::AccountAddressNotification, ([&accounts](
				const model::AccountAddressNotification& notification,
				const observers::ObserverContext& context) {
			CollectingAccountVisitor visitor(context, accounts);
			visitor.visit(notification.Address);
		}));
	}

	DECLARE_OBSERVER(HarvestingAccountPublicKey, model::AccountPublicKeyNotification)(HarvestingAffectedAccounts& accounts) {
		return MAKE_OBSERVER(HarvestingAccountPublicKey, model::AccountPublicKeyNotification, ([&accounts](
				const model::AccountPublicKeyNotification& notification,
				const observers::ObserverContext& context) {
			CollectingAccountVisitor visitor(context, accounts);
			visitor.visit(notification.PublicKey);
		}));
	}
}}
