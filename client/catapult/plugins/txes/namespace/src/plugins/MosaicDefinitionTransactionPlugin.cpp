#include "MosaicDefinitionTransactionPlugin.h"
#include "src/model/MosaicDefinitionTransaction.h"
#include "src/model/MosaicNotifications.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"
#include "catapult/constants.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		auto CreatePublisher(const MosaicRentalFeeConfiguration& config) {
			return [config](const TTransaction& transaction, NotificationSubscriber& sub) {
				// 1. sink account notification
				sub.notify(AccountPublicKeyNotification(config.SinkPublicKey));

				// 2. rental fee charge
				// a. exempt the nemesis account
				if (config.NemesisPublicKey != transaction.Signer)
					sub.notify(BalanceTransferNotification(transaction.Signer, config.SinkAddress, Xem_Id, config.Fee));

				// 3. registration
				sub.notify(MosaicNameNotification(
						transaction.MosaicId,
						transaction.ParentId,
						transaction.MosaicNameSize,
						transaction.NamePtr()));
				sub.notify(MosaicPropertiesNotification(transaction.PropertiesHeader, transaction.PropertiesPtr()));
				sub.notify(MosaicDefinitionNotification(
						transaction.Signer,
						transaction.ParentId,
						transaction.MosaicId,
						ExtractAllProperties(transaction.PropertiesHeader, transaction.PropertiesPtr())));
			};
		}
	}

	std::unique_ptr<TransactionPlugin> CreateMosaicDefinitionTransactionPlugin(const MosaicRentalFeeConfiguration& config) {
		return TransactionPluginFactory::Create<MosaicDefinitionTransaction, EmbeddedMosaicDefinitionTransaction>(
				CreatePublisher<MosaicDefinitionTransaction>(config),
				CreatePublisher<EmbeddedMosaicDefinitionTransaction>(config));
	}
}}
