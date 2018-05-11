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
