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

#include "NamespaceRegistrationTransactionPlugin.h"
#include "src/model/NamespaceNotifications.h"
#include "src/model/NamespaceRegistrationTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"
#include "catapult/constants.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		static void PublishBalanceTransfer(
				const NamespaceRentalFeeConfiguration& config,
				const TTransaction& transaction,
				const PublishContext& context,
				NotificationSubscriber& sub) {
			// a. exempt the nemesis account
			if (config.NemesisSignerPublicKey == transaction.SignerPublicKey)
				return;

			auto rentalFee = config.ChildFee;
			if (transaction.IsRootRegistration()) {
				// b. don't charge fees for eternal namespaces
				if (Eternal_Artifact_Duration == transaction.Duration)
					return;

				rentalFee = Amount(config.RootFeePerBlock.unwrap() * transaction.Duration.unwrap());
			}

			sub.notify(BalanceTransferNotification(
					context.SignerAddress,
					config.SinkAddress,
					config.CurrencyMosaicId,
					rentalFee,
					BalanceTransferNotification::AmountType::Dynamic));
			sub.notify(NamespaceRentalFeeNotification(context.SignerAddress, config.SinkAddress, config.CurrencyMosaicId, rentalFee));
		}

		template<typename TTransaction>
		auto CreatePublisher(const NamespaceRentalFeeConfiguration& config) {
			return [config](const TTransaction& transaction, const PublishContext& context, NotificationSubscriber& sub) {
				// 1. sink account notification
				sub.notify(AccountAddressNotification(config.SinkAddress));

				// 2. rental fee charge
				PublishBalanceTransfer(config, transaction, context, sub);

				// 3. registration notifications
				sub.notify(NamespaceRegistrationNotification(transaction.RegistrationType));
				auto parentId = Namespace_Base_Id;
				if (transaction.IsRootRegistration()) {
					using Notification = RootNamespaceNotification;
					sub.notify(Notification(context.SignerAddress, transaction.Id, transaction.Duration));
				} else {
					using Notification = ChildNamespaceNotification;
					sub.notify(Notification(context.SignerAddress, transaction.Id, transaction.ParentId));
					parentId = transaction.ParentId;
				}

				sub.notify(NamespaceNameNotification(transaction.Id, parentId, transaction.NameSize, transaction.NamePtr()));
			};
		}
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY_WITH_CONFIG(NamespaceRegistration, Default, CreatePublisher, NamespaceRentalFeeConfiguration)
}}
