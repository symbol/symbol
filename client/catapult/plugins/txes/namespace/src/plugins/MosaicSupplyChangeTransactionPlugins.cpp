#include "MosaicSupplyChangeTransactionPlugins.h"
#include "src/model/MosaicNotifications.h"
#include "src/model/MosaicSupplyChangeTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		void Publish(const TTransaction& transaction, NotificationSubscriber& sub) {
			sub.notify(MosaicChangeNotification(transaction.Signer, transaction.MosaicId));
			sub.notify(MosaicSupplyChangeNotification(
					transaction.Signer,
					transaction.MosaicId,
					transaction.Direction,
					transaction.Delta));
		}
	}

	std::unique_ptr<TransactionPlugin> CreateMosaicSupplyChangeTransactionPlugin() {
		return TransactionPluginFactory::Create<MosaicSupplyChangeTransaction, EmbeddedMosaicSupplyChangeTransaction>(
				Publish<MosaicSupplyChangeTransaction>,
				Publish<EmbeddedMosaicSupplyChangeTransaction>);
	}
}}
