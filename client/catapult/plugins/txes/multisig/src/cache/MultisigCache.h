#pragma once
#include "MultisigCacheDelta.h"
#include "MultisigCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	/// Cache composed of multisig information.
	using BasicMultisigCache = BasicCache<MultisigCacheDescriptor, MultisigCacheTypes::BaseSetType>;

	/// Synchronized cache composed of multisig information.
	class MultisigCache : public SynchronizedCache<BasicMultisigCache> {
	public:
		DEFINE_CACHE_CONSTANTS(Multisig)

	public:
		/// Creates a cache.
		MultisigCache() : SynchronizedCache<BasicMultisigCache>(BasicMultisigCache())
		{}
	};
}}
