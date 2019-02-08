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
#include "src/state/MosaicLevy.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	namespace {
		uint64_t GetPropertyValue(const model::MosaicProperties& properties, size_t index) {
			return (properties.begin() + index)->Value;
		}

		model::MosaicProperties MergeProperties(
				const model::MosaicProperties& currentProperties,
				const model::MosaicProperties& notificationProperties,
				NotifyMode mode) {
			model::MosaicProperties::PropertyValuesContainer propertyValues;
			for (auto i = 0u; i < model::First_Optional_Property; ++i)
				propertyValues[i] = GetPropertyValue(currentProperties, i) ^ GetPropertyValue(notificationProperties, i);

			auto durationIndex = model::First_Optional_Property;
			propertyValues[durationIndex] = NotifyMode::Commit == mode
					? GetPropertyValue(currentProperties, durationIndex) + GetPropertyValue(notificationProperties, durationIndex)
					: GetPropertyValue(currentProperties, durationIndex) - GetPropertyValue(notificationProperties, durationIndex);

			return model::MosaicProperties::FromValues(propertyValues);
		}

		auto ApplyNotification(
				state::MosaicEntry& currentEntry,
				const model::MosaicDefinitionNotification& notification,
				NotifyMode mode) {
			const auto& currentDefinition = currentEntry.definition();
			auto newProperties = MergeProperties(currentDefinition.properties(), notification.Properties, mode);
			auto revision = NotifyMode::Commit == mode ? currentDefinition.revision() + 1 : currentDefinition.revision() - 1;
			auto definition = state::MosaicDefinition(currentDefinition.height(), notification.Signer, revision, newProperties);
			state::MosaicEntry newEntry(notification.MosaicId, definition);
			if (currentEntry.hasLevy())
				CATAPULT_THROW_RUNTIME_ERROR("cannot observe mosaic entry with levy");

			return newEntry;
		}
	}

	DEFINE_OBSERVER(MosaicDefinition, model::MosaicDefinitionNotification, [](const auto& notification, const ObserverContext& context) {
		auto& cache = context.Cache.sub<cache::MosaicCache>();

		// mosaic supply will always be zero when a mosaic definition is observed
		auto mosaicIter = cache.find(notification.MosaicId);
		if (mosaicIter.tryGet()) {
			// copy existing mosaic entry before removing
			auto mosaicEntry = mosaicIter.get();
			cache.remove(notification.MosaicId);

			if (NotifyMode::Rollback == context.Mode && 1 == mosaicEntry.definition().revision())
				return;

			cache.insert(ApplyNotification(mosaicEntry, notification, context.Mode));
		} else {
			auto definition = state::MosaicDefinition(context.Height, notification.Signer, 1, notification.Properties);
			cache.insert(state::MosaicEntry(notification.MosaicId, definition));
		}
	});
}}
