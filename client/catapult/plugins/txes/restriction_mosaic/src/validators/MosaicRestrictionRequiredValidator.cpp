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
#include "src/cache/MosaicRestrictionCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	namespace {
		using Notification = model::MosaicRestrictionRequiredNotification;

		bool GlobalRestrictionExists(const Notification& notification, const ValidatorContext& context) {
			const auto& cache = context.Cache.sub<cache::MosaicRestrictionCache>();
			auto entryIter = cache.find(state::CreateMosaicRestrictionEntryKey(context.Resolvers.resolve(notification.MosaicId)));

			// entry must wrap a global restriction because key is created with zero address
			state::MosaicGlobalRestriction::RestrictionRule rule;
			return entryIter.tryGet() && entryIter.get().asGlobalRestriction().tryGet(notification.RestrictionKey, rule);
		}
	}

	DEFINE_STATEFUL_VALIDATOR(MosaicRestrictionRequired, ([](const Notification& notification, const ValidatorContext& context) {
		return GlobalRestrictionExists(notification, context)
				? ValidationResult::Success
				: Failure_RestrictionMosaic_Unknown_Global_Restriction;
	}))
}}
