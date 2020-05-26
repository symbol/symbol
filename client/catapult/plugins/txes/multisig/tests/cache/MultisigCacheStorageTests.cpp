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

#include "src/cache/MultisigCacheStorage.h"
#include "src/cache/MultisigCache.h"
#include "tests/test/MultisigTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		struct MultisigCacheStorageTraits {
			using StorageType = MultisigCacheStorage;
			class CacheType : public MultisigCache {
			public:
				CacheType() : MultisigCache(CacheConfiguration())
				{}
			};

			static auto CreateId(uint8_t id) {
				return Address{ { id } };
			}

			static auto CreateValue(const Address& address) {
				state::MultisigEntry entry(address);
				entry.setMinApproval(23);
				entry.setMinRemoval(34);

				for (auto i = 0u; i < 3u; ++i)
					entry.cosignatoryAddresses().insert(test::GenerateRandomByteArray<Address>());

				for (auto i = 0u; i < 4u; ++i)
					entry.multisigAddresses().insert(test::GenerateRandomByteArray<Address>());

				return entry;
			}

			static void AssertEqual(const state::MultisigEntry& lhs, const state::MultisigEntry& rhs) {
				test::AssertEqual(lhs, rhs);
			}
		};
	}

	DEFINE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TESTS(MultisigCacheStorageTests, MultisigCacheStorageTraits)
}}
