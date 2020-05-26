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

#include "Observers.h"
#include "src/cache/MosaicCache.h"

namespace catapult { namespace observers {

	namespace {
		model::MosaicFlags Xor(model::MosaicFlags lhs, model::MosaicFlags rhs) {
			return static_cast<model::MosaicFlags>(utils::to_underlying_type(lhs) ^ utils::to_underlying_type(rhs));
		}

		model::MosaicProperties MergeProperties(
				const model::MosaicProperties& currentProperties,
				const model::MosaicProperties& notificationProperties,
				NotifyMode mode) {
			auto flags = Xor(currentProperties.flags(), notificationProperties.flags());
			auto divisibility = static_cast<uint8_t>(currentProperties.divisibility() ^ notificationProperties.divisibility());
			auto duration = NotifyMode::Commit == mode
					? currentProperties.duration() + notificationProperties.duration()
					: currentProperties.duration() - notificationProperties.duration();
			return model::MosaicProperties(flags, divisibility, duration);
		}

		auto ApplyNotification(
				state::MosaicEntry& currentMosaicEntry,
				const model::MosaicDefinitionNotification& notification,
				NotifyMode mode) {
			const auto& currentDefinition = currentMosaicEntry.definition();
			auto newProperties = MergeProperties(currentDefinition.properties(), notification.Properties, mode);
			auto revision = NotifyMode::Commit == mode ? currentDefinition.revision() + 1 : currentDefinition.revision() - 1;
			auto definition = state::MosaicDefinition(currentDefinition.startHeight(), notification.Owner, revision, newProperties);
			return state::MosaicEntry(notification.MosaicId, definition);
		}
	}

	DEFINE_OBSERVER(MosaicDefinition, model::MosaicDefinitionNotification, [](
			const model::MosaicDefinitionNotification& notification,
			const ObserverContext& context) {
		auto& mosaicCache = context.Cache.sub<cache::MosaicCache>();

		// mosaic supply will always be zero when a mosaic definition is observed
		auto mosaicIter = mosaicCache.find(notification.MosaicId);
		if (mosaicIter.tryGet()) {
			// copy existing mosaic entry before removing
			auto mosaicEntry = mosaicIter.get();
			mosaicCache.remove(notification.MosaicId);

			if (NotifyMode::Rollback == context.Mode && 1 == mosaicEntry.definition().revision())
				return;

			mosaicCache.insert(ApplyNotification(mosaicEntry, notification, context.Mode));
		} else {
			auto definition = state::MosaicDefinition(context.Height, notification.Owner, 1, notification.Properties);
			mosaicCache.insert(state::MosaicEntry(notification.MosaicId, definition));
		}
	});
}}
