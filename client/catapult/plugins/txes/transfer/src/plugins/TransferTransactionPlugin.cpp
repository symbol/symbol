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

#include "TransferTransactionPlugin.h"
#include "src/model/TransferNotifications.h"
#include "src/model/TransferTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		template<typename TTransaction>
		void Publish(const TTransaction& transaction, const PublisherContext& publisherContext, NotificationSubscriber& sub) {
			auto recipient = publisherContext.resolve(transaction.Recipient);
			sub.notify(AccountAddressNotification(recipient));
			sub.notify(AddressInteractionNotification(transaction.Signer, { recipient }));

			const auto* pMosaics = transaction.MosaicsPtr();
			for (auto i = 0u; i < transaction.MosaicsCount; ++i) {
				auto notification = BalanceTransferNotification(
						transaction.Signer,
						recipient,
						publisherContext.resolve(pMosaics[i].MosaicId),
						pMosaics[i].Amount);
				sub.notify(notification);
			}

			if (transaction.MessageSize)
				sub.notify(TransferMessageNotification(transaction.MessageSize));

			if (transaction.MosaicsCount)
				sub.notify(TransferMosaicsNotification(transaction.MosaicsCount, pMosaics));
		}
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY(Transfer, Publish)
}}
