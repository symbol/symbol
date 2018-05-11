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

#include "HashLockTransactionPlugin.h"
#include "src/model/HashLockTransaction.h"
#include "src/model/LockNotifications.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		void Publish(const TTransaction& transaction, NotificationSubscriber& sub) {
			sub.notify(HashLockDurationNotification(transaction.Duration));
			sub.notify(HashLockMosaicNotification(transaction.Mosaic));

			// note that the BalanceReserveNotification has to be raised first since the HashLockNotification will trigger a debit
			sub.notify(BalanceReserveNotification(transaction.Signer, transaction.Mosaic.MosaicId, transaction.Mosaic.Amount));
			sub.notify(HashLockNotification(transaction.Signer, transaction.Mosaic, transaction.Duration, transaction.Hash));
		}
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY(HashLock, Publish)
}}
