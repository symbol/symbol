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

#include "Validators.h"
#include "ActiveMosaicView.h"
#include "src/cache/MosaicCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/validators/ValidatorContext.h"
#include "catapult/constants.h"

namespace catapult { namespace validators {

	using Notification = model::BalanceTransferNotification;

	namespace {
		bool IsMosaicOwnerParticipant(const cache::ReadOnlyCatapultCache& cache, const Key& owner, const Notification& notification) {
			if (owner == notification.Sender)
				return true;

			// the owner must exist if the mosaic lookup succeeded
			const auto& accountStateCache = cache.sub<cache::AccountStateCache>();
			auto ownerAccountStateIter = accountStateCache.find(owner);
			return ownerAccountStateIter.get().Address == notification.Recipient;
		}
	}

	DEFINE_STATEFUL_VALIDATOR(MosaicTransfer, [](const auto& notification, const auto& context) {
		// 0. whitelist xem
		if (Xem_Id == notification.MosaicId)
			return ValidationResult::Success;

		// 1. check that the mosaic exists
		ActiveMosaicView::FindIterator mosaicIter;
		ActiveMosaicView activeMosaicView(context.Cache);
		auto result = activeMosaicView.tryGet(notification.MosaicId, context.Height, mosaicIter);
		if (!IsValidationResultSuccess(result))
			return result;

		// 2. if it's transferable there's nothing else to check
		const auto& entry = mosaicIter.get();
		if (entry.definition().properties().is(model::MosaicFlags::Transferable))
			return ValidationResult::Success;

		// 3. if it's NOT transferable then owner must be either sender or recipient
		if (!IsMosaicOwnerParticipant(context.Cache, entry.definition().owner(), notification))
			return Failure_Mosaic_Non_Transferable;

		return ValidationResult::Success;
	});
}}
