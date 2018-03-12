#include "SecretLockTransactionPlugin.h"
#include "src/model/LockNotifications.h"
#include "src/model/SecretLockTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		auto CreateSecretLockNotification(const TTransaction& transaction) {
			return SecretLockNotification(
					transaction.Signer,
					transaction.Mosaic,
					transaction.Duration,
					transaction.HashAlgorithm,
					transaction.Secret,
					transaction.Recipient);
		}

		template<typename TTransaction>
		void Publish(const TTransaction& transaction, NotificationSubscriber& sub) {
			sub.notify(AccountAddressNotification(transaction.Recipient));
			sub.notify(SecretLockDurationNotification(transaction.Duration));
			sub.notify(SecretLockHashAlgorithmNotification(transaction.HashAlgorithm));
			sub.notify(CreateSecretLockNotification(transaction));

			BalanceTransferNotification balanceTransferNotification(
					transaction.Signer,
					transaction.Recipient,
					transaction.Mosaic.MosaicId,
					transaction.Mosaic.Amount);
			model::SetNotificationChannel(balanceTransferNotification.Type, model::NotificationChannel::Validator);
			sub.notify(balanceTransferNotification);
		}
	}

	std::unique_ptr<TransactionPlugin> CreateSecretLockTransactionPlugin() {
		return TransactionPluginFactory::Create<SecretLockTransaction, EmbeddedSecretLockTransaction>(
				Publish<SecretLockTransaction>,
				Publish<EmbeddedSecretLockTransaction>);
	}
}}
