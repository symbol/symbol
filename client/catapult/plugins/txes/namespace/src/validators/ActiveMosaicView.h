#pragma once
#include "catapult/validators/ValidationResult.h"
#include "catapult/types.h"

namespace catapult {
	namespace cache { class ReadOnlyCatapultCache; }
	namespace state { class MosaicEntry; }
}

namespace catapult { namespace validators {

	/// A view on top of a catapult cache cache for retrieving active mosaics.
	class ActiveMosaicView {
	public:
		/// Creates a view around \a cache.
		explicit ActiveMosaicView(const cache::ReadOnlyCatapultCache& cache) : m_cache(cache)
		{}

	public:
		/// Tries to get an entry (\a pEntry) for an active mosaic with \a id at \a height.
		validators::ValidationResult tryGet(MosaicId id, Height height, const state::MosaicEntry** pEntry) const;

		/// Tries to get an entry (\a pEntry) for an active mosaic with \a id at \a height given its purported \a owner.
		validators::ValidationResult tryGet(MosaicId id, Height height, const Key& owner, const state::MosaicEntry** pEntry) const;

	private:
		const cache::ReadOnlyCatapultCache& m_cache;
	};
}}
