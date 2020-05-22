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

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		auto CreatePublisher(const MosaicRentalFeeConfiguration& config) {
			return [config](const TTransaction& transaction, const PublishContext& context, NotificationSubscriber& sub) {
				// 1. sink account notification
				sub.notify(AccountAddressNotification(config.SinkAddress));

				// 2. rental fee charge
				// a. exempt the nemesis account
				if (config.NemesisSignerPublicKey != transaction.SignerPublicKey) {
					sub.notify(BalanceTransferNotification(
							context.SignerAddress,
							config.SinkAddress,
							config.CurrencyMosaicId,
							config.Fee,
							BalanceTransferNotification::AmountType::Dynamic));
					sub.notify(MosaicRentalFeeNotification(
							context.SignerAddress,
							config.SinkAddress,
							config.CurrencyMosaicId,
							config.Fee));
				}

				// 3. registration
				auto properties = MosaicProperties(transaction.Flags, transaction.Divisibility, transaction.Duration);
				sub.notify(MosaicNonceNotification(context.SignerAddress, transaction.Nonce, transaction.Id));
				sub.notify(MosaicPropertiesNotification(properties));
				sub.notify(MosaicDefinitionNotification(context.SignerAddress, transaction.Id, properties));
			};
		}
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY_WITH_CONFIG(MosaicDefinition, Default, CreatePublisher, MosaicRentalFeeConfiguration)
}}
