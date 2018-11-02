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
			return [config](const TTransaction& transaction, const PublisherContext&, NotificationSubscriber& sub) {
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

	DEFINE_TRANSACTION_PLUGIN_FACTORY_WITH_CONFIG(MosaicDefinition, CreatePublisher, MosaicRentalFeeConfiguration)
}}
