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
#include "catapult/model/EntityType.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace observers {

	namespace {
		class DefaultAccountVisitor {
		public:
			explicit DefaultAccountVisitor(const ObserverContext& context) : m_context(context)
			{}

		public:
			void visit(const Address& address) {
				notify(address);
			}

			void visit(const Key& publicKey) {
				notify(publicKey);
			}

		private:
			template<typename AccountId>
			void notify(const AccountId& accountId) {
				auto& accountStateCache = m_context.Cache.sub<cache::AccountStateCache>();
				if (NotifyMode::Commit == m_context.Mode)
					accountStateCache.addAccount(accountId, m_context.Height);
				else
					accountStateCache.queueRemove(accountId, m_context.Height);
			}

		private:
			const ObserverContext& m_context;
		};
	}

	DEFINE_OBSERVER(AccountAddress, model::AccountAddressNotification, [](const auto& notification, const auto& context) {
		DefaultAccountVisitor visitor(context);
		visitor.visit(notification.Address);
	});

	DEFINE_OBSERVER(AccountPublicKey, model::AccountPublicKeyNotification, [](const auto& notification, const auto& context) {
		DefaultAccountVisitor visitor(context);
		visitor.visit(notification.PublicKey);
	});
}}
