#include "TransferTransactionPlugins.h"
#include "src/model/TransferNotifications.h"
#include "src/model/TransferTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		void Publish(const TTransaction& transaction, NotificationSubscriber& sub) {
			sub.notify(AccountAddressNotification(transaction.Recipient));

			const auto* pMosaics = transaction.MosaicsPtr();
			for (auto i = 0u; i < transaction.MosaicsCount; ++i) {
				auto notification = BalanceTransferNotification(
						transaction.Signer,
						transaction.Recipient,
						pMosaics[i].MosaicId,
						pMosaics[i].Amount);
				sub.notify(notification);
			}

			if (transaction.MessageSize)
				sub.notify(TransferMessageNotification(transaction.MessageSize));

			if (transaction.MosaicsCount)
				sub.notify(TransferMosaicsNotification(transaction.MosaicsCount, pMosaics));
		}
	}

	std::unique_ptr<TransactionPlugin> CreateTransferTransactionPlugin() {
		return TransactionPluginFactory::Create<TransferTransaction, EmbeddedTransferTransaction>(
				Publish<TransferTransaction>,
				Publish<EmbeddedTransferTransaction>);
	}
}}
