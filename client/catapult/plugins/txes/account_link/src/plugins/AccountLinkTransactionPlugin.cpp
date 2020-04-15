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

#include "AccountLinkTransactionPlugin.h"
#include "src/model/AccountLinkNotifications.h"
#include "src/model/AccountLinkTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		void Publish(const TTransaction& transaction, NotificationSubscriber& sub) {
			if (LinkAction::Link == transaction.LinkAction) {
				// NewRemoteAccountNotification must be raised before AccountPublicKeyNotification because the latter adds account to cache
				sub.notify(NewRemoteAccountNotification(transaction.RemotePublicKey));
				sub.notify(AccountPublicKeyNotification(transaction.RemotePublicKey));
			}

			sub.notify(AddressInteractionNotification(transaction.SignerPublicKey, transaction.Type, {}, { transaction.RemotePublicKey }));
			sub.notify(RemoteAccountLinkNotification(transaction.SignerPublicKey, transaction.RemotePublicKey, transaction.LinkAction));
		}
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY(AccountLink, Default, Publish)
}}
