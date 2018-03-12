#pragma once
#include "MultisigCacheTypes.h"
#include "catapult/utils/ArraySet.h"

namespace catapult { namespace cache {

	/// Finds all ancestors of a \a key in \a cache, adds them to \a ancestorKeys and returns the maximum distance between
	/// \a key and any ancestor.
	size_t FindAncestors(const MultisigCacheTypes::CacheReadOnlyType& cache, const Key& key, utils::KeySet& ancestorKeys);

	/// Finds all descendants of a \a key in \a cache, adds them to \a descendantKeys and returns the maximum distance between
	/// \a key and any descendant.
	size_t FindDescendants(const MultisigCacheTypes::CacheReadOnlyType& cache, const Key& key, utils::KeySet& descendantKeys);
}}
