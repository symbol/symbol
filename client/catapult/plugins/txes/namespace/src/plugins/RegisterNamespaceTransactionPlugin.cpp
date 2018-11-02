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

#include "RegisterNamespaceTransactionPlugin.h"
#include "src/model/NamespaceNotifications.h"
#include "src/model/RegisterNamespaceTransaction.h"
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
				NotificationSubscriber& sub) {
			// a. exempt the nemesis account
			if (config.NemesisPublicKey == transaction.Signer)
				return;

			auto rentalFee = config.ChildFee;
			if (transaction.IsRootRegistration()) {
				// b. don't charge fees for eternal namespaces
				if (Eternal_Artifact_Duration == transaction.Duration)
					return;

				rentalFee = Amount(config.RootFeePerBlock.unwrap() * transaction.Duration.unwrap());
			}

			sub.notify(BalanceTransferNotification(transaction.Signer, config.SinkAddress, Xem_Id, rentalFee));
		}

		template<typename TTransaction>
		auto CreatePublisher(const NamespaceRentalFeeConfiguration& config) {
			return [config](const TTransaction& transaction, const PublisherContext&, NotificationSubscriber& sub) {
				// 1. sink account notification
				sub.notify(AccountPublicKeyNotification(config.SinkPublicKey));

				// 2. rental fee charge
				PublishBalanceTransfer(config, transaction, sub);

				// 3. registration notifications
				sub.notify(NamespaceNotification(transaction.NamespaceType));
				auto parentId = Namespace_Base_Id;
				if (transaction.IsRootRegistration()) {
					using Notification = RootNamespaceNotification;
					sub.notify(Notification(transaction.Signer, transaction.NamespaceId, transaction.Duration));
				} else {
					using Notification = ChildNamespaceNotification;
					sub.notify(Notification(transaction.Signer, transaction.NamespaceId, transaction.ParentId));
					parentId = transaction.ParentId;
				}

				sub.notify(NamespaceNameNotification(
						transaction.NamespaceId,
						parentId,
						transaction.NamespaceNameSize,
						transaction.NamePtr()));
			};
		}
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY_WITH_CONFIG(RegisterNamespace, CreatePublisher, NamespaceRentalFeeConfiguration)
}}
