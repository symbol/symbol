/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#pragma once
#include "ObserverTypes.h"
#include "catapult/model/Receipt.h"

namespace catapult { namespace observers {

	/// Creates a rental fee observer with \a name that adds receipts with \a receiptType.
	template<typename TNotification>
	NotificationObserverPointerT<TNotification> CreateRentalFeeObserver(const std::string& name, model::ReceiptType receiptType) {
		using ObserverType = FunctionalNotificationObserverT<TNotification>;
		return std::make_unique<ObserverType>(name + "RentalFeeObserver", [receiptType](
				const auto& notification,
				ObserverContext& context) {
			if (NotifyMode::Rollback == context.Mode)
				return;

			auto mosaicId = context.Resolvers.resolve(notification.MosaicId);
			auto recipient = context.Resolvers.resolve(notification.Recipient);
			auto effectiveAmount = Amount(notification.Amount.unwrap() * context.Cache.dependentState().DynamicFeeMultiplier.unwrap());
			model::BalanceTransferReceipt receipt(receiptType, notification.Sender, recipient, mosaicId, effectiveAmount);
			context.StatementBuilder().addReceipt(receipt);
		});
	}
}}
