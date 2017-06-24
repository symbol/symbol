#include "ReadOnlyCatapultCache.h"
#include "CatapultCache.h"

namespace catapult { namespace cache {

	ReadOnlyCatapultCache::ReadOnlyCatapultCache(const std::vector<const void*>& readOnlyViews) : m_readOnlyViews(readOnlyViews)
	{}
}}
