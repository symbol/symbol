#include "SecretProofTransactionPlugin.h"
#include "src/model/LockNotifications.h"
#include "src/model/SecretProofTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		void Publish(const TTransaction& transaction, NotificationSubscriber& sub) {
			sub.notify(SecretLockHashAlgorithmNotification(transaction.HashAlgorithm));
			sub.notify(ProofSecretNotification(
					transaction.HashAlgorithm,
					transaction.Secret,
					{ transaction.ProofPtr(), transaction.ProofSize }));
			sub.notify(ProofPublicationNotification(transaction.Signer, transaction.HashAlgorithm, transaction.Secret));
		}
	}

	std::unique_ptr<TransactionPlugin> CreateSecretProofTransactionPlugin() {
		return TransactionPluginFactory::Create<SecretProofTransaction, EmbeddedSecretProofTransaction>(
				Publish<SecretProofTransaction>,
				Publish<EmbeddedSecretProofTransaction>);
	}
}}
