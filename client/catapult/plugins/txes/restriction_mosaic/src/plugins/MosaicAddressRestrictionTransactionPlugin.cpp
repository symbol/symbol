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

#include "MosaicAddressRestrictionTransactionPlugin.h"
#include "src/model/MosaicAddressRestrictionTransaction.h"
#include "src/model/MosaicRestrictionNotifications.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		constexpr uint8_t Mosaic_Flags_Restrictable = 0x04;

		template<typename TTransaction>
		void Publish(const TTransaction& transaction, const PublishContext& context, NotificationSubscriber& sub) {
			// conceptually, mosaic restrictions are mosaic, not account, settings
			// don't raise AddressInteractionNotification between Signer and TargetAddress because mosaic owner always needs to
			// be able to restrict or allow mosaic usage by *any* account

			sub.notify(MosaicRequiredNotification(context.SignerAddress, transaction.MosaicId, Mosaic_Flags_Restrictable));
			sub.notify(MosaicRestrictionRequiredNotification(transaction.MosaicId, transaction.RestrictionKey));
			sub.notify(MosaicAddressRestrictionModificationPreviousValueNotification(
					transaction.MosaicId,
					transaction.RestrictionKey,
					transaction.TargetAddress,
					transaction.PreviousRestrictionValue));
			sub.notify(MosaicAddressRestrictionModificationNewValueNotification(
					transaction.MosaicId,
					transaction.RestrictionKey,
					transaction.TargetAddress,
					transaction.NewRestrictionValue));
		}
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY(MosaicAddressRestriction, Default, Publish)
}}
