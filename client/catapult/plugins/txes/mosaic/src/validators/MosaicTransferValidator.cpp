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

namespace catapult { namespace validators {

	using Notification = model::BalanceTransferNotification;

	namespace {
		bool IsMosaicOwnerParticipant(
				const cache::ReadOnlyCatapultCache& cache,
				const Address& owner,
				const Notification& notification,
				const model::ResolverContext& resolvers) {
			if (owner == notification.Sender)
				return true;

			// the owner must exist if the mosaic lookup succeeded
			const auto& accountStateCache = cache.sub<cache::AccountStateCache>();
			auto ownerAccountStateIter = accountStateCache.find(owner);
			return ownerAccountStateIter.get().Address == resolvers.resolve(notification.Recipient);
		}
	}

	DECLARE_STATEFUL_VALIDATOR(MosaicTransfer, Notification)(UnresolvedMosaicId currencyMosaicId) {
		return MAKE_STATEFUL_VALIDATOR(MosaicTransfer, [currencyMosaicId](
				const Notification& notification,
				const ValidatorContext& context) {
			// 0. allow currency mosaic id
			if (currencyMosaicId == notification.MosaicId)
				return ValidationResult::Success;

			// 1. check that the mosaic exists
			ActiveMosaicView::FindIterator mosaicIter;
			ActiveMosaicView activeMosaicView(context.Cache);
			auto result = activeMosaicView.tryGet(context.Resolvers.resolve(notification.MosaicId), context.Height, mosaicIter);
			if (!IsValidationResultSuccess(result))
				return result;

			// 2. if it's transferable there's nothing else to check
			const auto& mosaicEntry = mosaicIter.get();
			if (mosaicEntry.definition().properties().is(model::MosaicFlags::Transferable))
				return ValidationResult::Success;

			// 3. if it's NOT transferable then owner must be either sender or recipient
			if (!IsMosaicOwnerParticipant(context.Cache, mosaicEntry.definition().ownerAddress(), notification, context.Resolvers))
				return Failure_Mosaic_Non_Transferable;

			return ValidationResult::Success;
		});
	}
}}
