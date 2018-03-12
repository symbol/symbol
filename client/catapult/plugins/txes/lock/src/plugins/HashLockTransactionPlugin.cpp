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

	std::unique_ptr<TransactionPlugin> CreateHashLockTransactionPlugin() {
		return TransactionPluginFactory::Create<HashLockTransaction, EmbeddedHashLockTransaction>(
				Publish<HashLockTransaction>,
				Publish<EmbeddedHashLockTransaction>);
	}
}}
