#include "MultisigCacheUtils.h"
#include "MultisigCache.h"

namespace catapult { namespace cache {

	namespace {
		struct AncestorTraits {
			static const auto& GetKeySet(const state::MultisigEntry& multisigEntry) {
				return multisigEntry.multisigAccounts();
			}
		};

		struct DescendantTraits {
			static const auto& GetKeySet(const state::MultisigEntry& multisigEntry) {
				return multisigEntry.cosignatories();
			}
		};

		template<typename TTraits>
		size_t FindAll(const MultisigCacheTypes::CacheReadOnlyType& multisigCache, const Key& publicKey, utils::KeySet& keySet) {
			if (!multisigCache.contains(publicKey))
				return 0;

			size_t numLevels = 0;
			const auto& multisigEntry = multisigCache.get(publicKey);
			for (const auto& linkedKey : TTraits::GetKeySet(multisigEntry)) {
				keySet.insert(linkedKey);
				numLevels = std::max(numLevels, FindAll<TTraits>(multisigCache, linkedKey, keySet) + 1);
			}

			return numLevels;
		}
	}

	size_t FindAncestors(const MultisigCacheTypes::CacheReadOnlyType& cache, const Key& key, utils::KeySet& ancestorKeys) {
		return FindAll<AncestorTraits>(cache, key, ancestorKeys);
	}

	size_t FindDescendants(const MultisigCacheTypes::CacheReadOnlyType& cache, const Key& key, utils::KeySet& descendantKeys) {
		return FindAll<DescendantTraits>(cache, key, descendantKeys);
	}
}}
