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

#pragma once
#include "src/cache/MosaicCache.h"
#include "catapult/validators/ValidationResult.h"
#include "catapult/types.h"

namespace catapult {
	namespace cache { class ReadOnlyCatapultCache; }
	namespace state { class MosaicEntry; }
}

namespace catapult { namespace validators {

	/// View on top of a catapult cache cache for retrieving active mosaics.
	class ActiveMosaicView {
	public:
		/// Creates a view around \a cache.
		explicit ActiveMosaicView(const cache::ReadOnlyCatapultCache& cache);

	public:
		/// Iterator type returned by tryGet.
		using FindIterator = cache::MosaicCacheTypes::CacheReadOnlyType::ReadOnlyFindIterator<
			cache::MosaicCacheView::const_iterator,
			cache::MosaicCacheDelta::const_iterator
		>;

	public:
		/// Tries to get an entry iterator (\a iter) for an active mosaic with \a id at \a height.
		validators::ValidationResult tryGet(MosaicId id, Height height, FindIterator& iter) const;

		/// Tries to get an entry iterator (\a iter) for an active mosaic with \a id at \a height given its purported \a owner.
		validators::ValidationResult tryGet(MosaicId id, Height height, const Address& owner, FindIterator& iter) const;

	private:
		const cache::ReadOnlyCatapultCache& m_cache;
	};
}}
