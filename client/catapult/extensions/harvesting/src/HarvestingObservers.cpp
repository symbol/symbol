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

#include "HarvestingObservers.h"

namespace catapult { namespace harvesting {

	namespace {
		template<typename TAccountIdentifier>
		class CollectingAccountVisitor {
		public:
			CollectingAccountVisitor(
					const observers::ObserverContext& context,
					RefCountedAccountIdentifiers<TAccountIdentifier>& accountIdentifiers)
					: m_context(context)
					, m_accountIdentifiers(accountIdentifiers)
			{}

		public:
			void visit(const model::ResolvableAddress& address) {
				notify(address.resolved(m_context.Resolvers));
			}

			void visit(const Key& publicKey) {
				notify(publicKey);
			}

		private:
			void notify(const TAccountIdentifier& accountIdentifier) {
				auto iter = m_accountIdentifiers.find(accountIdentifier);
				if (observers::NotifyMode::Commit == m_context.Mode) {
					if (m_accountIdentifiers.cend() == iter)
						m_accountIdentifiers.emplace(accountIdentifier, 1);
					else
						++iter->second;

					return;
				}

				// rollback - so account must have been previously added
				if (0 == --iter->second)
					m_accountIdentifiers.erase(iter);
			}

		private:
			const observers::ObserverContext& m_context;
			RefCountedAccountIdentifiers<TAccountIdentifier>& m_accountIdentifiers;
		};
	}

	DECLARE_OBSERVER(HarvestingAccountAddress, model::AccountAddressNotification)(RefCountedAccountIdentifiers<Address>& addresses) {
		return MAKE_OBSERVER(HarvestingAccountAddress, model::AccountAddressNotification, ([&addresses](
				const model::AccountAddressNotification& notification,
				const observers::ObserverContext& context) {
			CollectingAccountVisitor<Address> visitor(context, addresses);
			visitor.visit(notification.Address);
		}));
	}

	DECLARE_OBSERVER(HarvestingAccountPublicKey, model::AccountPublicKeyNotification)(RefCountedAccountIdentifiers<Key>& publicKeys) {
		return MAKE_OBSERVER(HarvestingAccountPublicKey, model::AccountPublicKeyNotification, ([&publicKeys](
				const model::AccountPublicKeyNotification& notification,
				const observers::ObserverContext& context) {
			CollectingAccountVisitor<Key> visitor(context, publicKeys);
			visitor.visit(notification.PublicKey);
		}));
	}
}}
