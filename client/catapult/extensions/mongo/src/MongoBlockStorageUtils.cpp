#include "MongoBlockStorageUtils.h"
#include "catapult/io/BlockStorage.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/TransactionUtils.h"

namespace catapult { namespace mongo {

	void PrepareMongoBlockStorage(
			io::LightBlockStorage& destinationStorage,
			const io::BlockStorage& sourceStorage,
			const model::NotificationPublisher& notificationPublisher) {
		// make a copy of the nemesis block to avoid modifying the original block element
		CATAPULT_LOG(info) << "initializing storage with nemesis data";
		auto pSourceNemesisBlock = sourceStorage.loadBlockElement(Height(1));
		auto nemesisBlock = *pSourceNemesisBlock;
		for (auto& transactionElement : nemesisBlock.Transactions) {
			auto addresses = model::ExtractAddresses(transactionElement.Transaction, notificationPublisher);
			transactionElement.OptionalExtractedAddresses = std::make_shared<model::AddressSet>(std::move(addresses));
		}

		destinationStorage.saveBlock(nemesisBlock);
	}
}}
