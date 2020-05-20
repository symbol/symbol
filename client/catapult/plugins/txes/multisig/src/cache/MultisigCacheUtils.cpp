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
			static const auto& GetAddresses(const state::MultisigEntry& multisigEntry) {
				return multisigEntry.multisigAddresses();
			}
		};

		struct DescendantTraits {
			static const auto& GetAddresses(const state::MultisigEntry& multisigEntry) {
				return multisigEntry.cosignatoryAddresses();
			}
		};

		template<typename TTraits>
		size_t FindAll(const MultisigCacheTypes::CacheReadOnlyType& multisigCache, const Address& address, model::AddressSet& addresses) {
			auto multisigIter = multisigCache.find(address);
			if (!multisigIter.tryGet())
				return 0;

			size_t numLevels = 0;
			const auto& multisigEntry = multisigIter.get();
			for (const auto& linkedAddress : TTraits::GetAddresses(multisigEntry)) {
				addresses.insert(linkedAddress);
				numLevels = std::max(numLevels, FindAll<TTraits>(multisigCache, linkedAddress, addresses) + 1);
			}

			return numLevels;
		}
	}

	size_t FindAncestors(const MultisigCacheTypes::CacheReadOnlyType& cache, const Address& address, model::AddressSet& ancestors) {
		return FindAll<AncestorTraits>(cache, address, ancestors);
	}

	size_t FindDescendants(const MultisigCacheTypes::CacheReadOnlyType& cache, const Address& address, model::AddressSet& descendants) {
		return FindAll<DescendantTraits>(cache, address, descendants);
	}
}}
