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

#include "AccountRestrictionView.h"
#include "Validators.h"
#include "src/cache/AccountRestrictionCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::BalanceTransferNotification;

	DEFINE_STATEFUL_VALIDATOR(MosaicRecipient, [](const Notification& notification, const ValidatorContext& context) {
		AccountRestrictionView view(context.Cache);
		if (!view.initialize(notification.Recipient.resolved(context.Resolvers)))
			return ValidationResult::Success;

		auto mosaicId = context.Resolvers.resolve(notification.MosaicId);
		auto isTransferAllowed = view.isAllowed(model::AccountRestrictionFlags::MosaicId, mosaicId);
		return isTransferAllowed ? ValidationResult::Success : Failure_RestrictionAccount_Mosaic_Transfer_Prohibited;
	})
}}
