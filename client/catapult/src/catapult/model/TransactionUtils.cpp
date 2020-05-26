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

#include "TransactionUtils.h"
#include "Address.h"
#include "NotificationPublisher.h"
#include "NotificationSubscriber.h"
#include "ResolverContext.h"
#include "Transaction.h"

namespace catapult { namespace model {

	namespace {
		class AddressCollector : public NotificationSubscriber {
		public:
			explicit AddressCollector(NetworkIdentifier networkIdentifier) : m_networkIdentifier(networkIdentifier)
			{}

		public:
			void notify(const Notification& notification) override {
				if (Core_Register_Account_Address_Notification == notification.Type)
					m_addresses.insert(static_cast<const AccountAddressNotification&>(notification).Address.unresolved());
				else if (Core_Register_Account_Public_Key_Notification == notification.Type)
					m_addresses.insert(toAddress(static_cast<const AccountPublicKeyNotification&>(notification).PublicKey));
			}

		public:
			const UnresolvedAddressSet& addresses() const {
				return m_addresses;
			}

		private:
			UnresolvedAddress toAddress(const Key& publicKey) const {
				auto resolvedAddress = PublicKeyToAddress(publicKey, m_networkIdentifier);
				return resolvedAddress.copyTo<UnresolvedAddress>();
			}

		private:
			NetworkIdentifier m_networkIdentifier;
			UnresolvedAddressSet m_addresses;
		};
	}

	UnresolvedAddressSet ExtractAddresses(const Transaction& transaction, const NotificationPublisher& notificationPublisher) {
		Hash256 transactionHash;
		WeakEntityInfo weakInfo(transaction, transactionHash);
		AddressCollector sub(NetworkIdentifier(weakInfo.entity().Network));
		notificationPublisher.publish(weakInfo, sub);
		return sub.addresses();
	}
}}
