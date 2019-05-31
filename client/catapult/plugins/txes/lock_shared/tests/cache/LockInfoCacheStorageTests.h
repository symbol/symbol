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

#pragma once
#include "plugins/txes/lock_shared/tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/cache/CacheStorageTestUtils.h"

namespace catapult { namespace cache {

	/// Lock info cache storage test traits.
	template<typename TLockInfoTraits>
	struct LockInfoCacheStorageTraits {
		using StorageType = typename TLockInfoTraits::StorageType;
		class CacheType : public TLockInfoTraits::CacheType {
		public:
			CacheType() : TLockInfoTraits::CacheType(CacheConfiguration())
			{}
		};

		static auto CreateId(uint8_t id) {
			return typename TLockInfoTraits::KeyType{ { id } };
		}

		static auto CreateValue(const typename TLockInfoTraits::KeyType& key) {
			auto lockInfo = TLockInfoTraits::CreateLockInfo(Height(1));
			TLockInfoTraits::SetKey(lockInfo, key);
			return lockInfo;
		}

		static void AssertEqual(const typename TLockInfoTraits::ValueType& lhs, const typename TLockInfoTraits::ValueType& rhs) {
			TLockInfoTraits::AssertEqual(lhs, rhs);
		}
	};
}}

#define DEFINE_LOCK_INFO_CACHE_STORAGE_TESTS(TRAITS_NAME) \
	DEFINE_BASIC_INSERT_REMOVE_CACHE_STORAGE_TESTS(TEST_CLASS, LockInfoCacheStorageTraits<TRAITS_NAME>)
