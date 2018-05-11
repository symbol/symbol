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
