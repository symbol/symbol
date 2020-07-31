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
#include "src/cache/MetadataCache.h"

namespace catapult { namespace observers {

	namespace {
		void UpdateCache(cache::MetadataCacheDelta& cache, const state::MetadataKey& metadataKey, const RawBuffer& valueBuffer) {
			auto metadataIter = cache.find(metadataKey.uniqueKey());

			if (!metadataIter.tryGet()) {
				auto metadataEntry = state::MetadataEntry(metadataKey);
				metadataEntry.value().update(valueBuffer);
				cache.insert(metadataEntry);
				return;
			}

			if (0 == valueBuffer.Size)
				cache.remove(metadataKey.uniqueKey());
			else
				metadataIter.get().value().update(valueBuffer);
		}
	}

	DEFINE_OBSERVER(MetadataValue, model::MetadataValueNotification, [](
			const model::MetadataValueNotification& notification,
			const ObserverContext& context) {
		auto& cache = context.Cache.sub<cache::MetadataCache>();

		int32_t valueSize = notification.ValueSize;
		if (NotifyMode::Commit == context.Mode) {
			if (notification.ValueSizeDelta < 0)
				valueSize += notification.ValueSizeDelta;
		} else {
			if (notification.ValueSizeDelta > 0)
				valueSize -= notification.ValueSizeDelta;
		}

		auto metadataKey = state::ResolveMetadataKey(notification.PartialMetadataKey, notification.MetadataTarget, context.Resolvers);
		UpdateCache(cache, metadataKey, { notification.ValuePtr, static_cast<size_t>(valueSize) });
	})
}}
