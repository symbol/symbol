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

#include "MosaicGlobalRestrictionTransactionPlugin.h"
#include "src/model/MosaicGlobalRestrictionTransaction.h"
#include "src/model/MosaicRestrictionNotifications.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionPluginFactory.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

	namespace {
		constexpr uint8_t Mosaic_Flags_Restrictable = 0x04;

		template<typename TTransaction>
		void Publish(const TTransaction& transaction, const PublishContext& context, NotificationSubscriber& sub) {
			sub.notify(MosaicRestrictionTypeNotification(transaction.NewRestrictionType));

			sub.notify(MosaicRequiredNotification(context.SignerAddress, transaction.MosaicId, Mosaic_Flags_Restrictable));

			if (UnresolvedMosaicId() != transaction.ReferenceMosaicId)
				sub.notify(MosaicRestrictionRequiredNotification(transaction.ReferenceMosaicId, transaction.RestrictionKey));

			sub.notify(MosaicGlobalRestrictionModificationPreviousValueNotification(
					transaction.MosaicId,
					transaction.ReferenceMosaicId,
					transaction.RestrictionKey,
					transaction.PreviousRestrictionValue,
					transaction.PreviousRestrictionType));
			sub.notify(MosaicGlobalRestrictionModificationNewValueNotification(
					transaction.MosaicId,
					transaction.ReferenceMosaicId,
					transaction.RestrictionKey,
					transaction.NewRestrictionValue,
					transaction.NewRestrictionType));
		}
	}

	DEFINE_TRANSACTION_PLUGIN_FACTORY(MosaicGlobalRestriction, Default, Publish)
}}
