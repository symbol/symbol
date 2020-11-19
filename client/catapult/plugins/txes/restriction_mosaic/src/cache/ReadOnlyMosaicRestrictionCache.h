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
#include "src/state/MosaicRestrictionEntry.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/model/NetworkIdentifier.h"

namespace catapult {
	namespace cache {
		class BasicMosaicRestrictionCacheDelta;
		class BasicMosaicRestrictionCacheView;
	}
}

namespace catapult { namespace cache {

	using ReadOnlyMosaicRestrictionArtifactCache = ReadOnlyArtifactCache<
		BasicMosaicRestrictionCacheView,
		BasicMosaicRestrictionCacheDelta,
		Hash256,
		state::MosaicRestrictionEntry>;

	/// Read-only overlay on top of a mosaic restriction cache.
	class ReadOnlyMosaicRestrictionCache : public ReadOnlyMosaicRestrictionArtifactCache {
	public:
		/// Creates a read-only overlay on top of \a cache.
		explicit ReadOnlyMosaicRestrictionCache(const BasicMosaicRestrictionCacheView& cache);

		/// Creates a read-only overlay on top of \a cache.
		explicit ReadOnlyMosaicRestrictionCache(const BasicMosaicRestrictionCacheDelta& cache);

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const;

	private:
		const BasicMosaicRestrictionCacheView* m_pCache;
		const BasicMosaicRestrictionCacheDelta* m_pCacheDelta;
	};
}}
