#pragma once
#include "NamespaceCacheDelta.h"
#include "NamespaceCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	/// Cache composed of namespace information.
	using BasicNamespaceCache = BasicCache<NamespaceCacheDescriptor, NamespaceCacheTypes::BaseSetType>;

	/// Synchronized cache composed of namespace information.
	class NamespaceCache : public SynchronizedCache<BasicNamespaceCache> {
	public:
		DEFINE_CACHE_CONSTANTS(Namespace)

	public:
		/// Creates a cache.
		NamespaceCache() : SynchronizedCache<BasicNamespaceCache>(BasicNamespaceCache())
		{}
	};
}}
