/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "src/cache/AccountRestrictionCacheStorage.h"
#include "src/cache/AccountRestrictionCache.h"
#include "src/model/AccountRestrictionFlags.h"
#include "tests/test/AccountRestrictionTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

	namespace {
		struct AccountRestrictionCacheStorageTraits {
			using StorageType = AccountRestrictionCacheStorage;
			class CacheType : public AccountRestrictionCache {
			public:
				CacheType() : AccountRestrictionCache(CacheConfiguration(), static_cast<model::NetworkIdentifier>(12))
				{}
			};

			static auto CreateId(uint8_t id) {
				return Address{ { id } };
			}

			static auto CreateValue(const Address& address) {
				using ModificationAction = model::AccountRestrictionModificationAction;

				state::AccountRestrictions restrictions(address);
				auto& restriction = restrictions.restriction(model::AccountRestrictionFlags::Address);
				for (auto i = 0u; i < 3; ++i)
					restriction.allow({ ModificationAction::Add, test::GenerateRandomVector(Address::Size) });

				return restrictions;
			}

			static void AssertEqual(const state::AccountRestrictions& lhs, const state::AccountRestrictions& rhs) {
				test::AssertEqual(lhs, rhs);
			}
		};
	}

	DEFINE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TESTS(AccountRestrictionCacheStorageTests, AccountRestrictionCacheStorageTraits)
}}
