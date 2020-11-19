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

#pragma once
#include "src/cache/HashLockInfoCache.h"
#include "src/cache/HashLockInfoCacheStorage.h"
#include "src/cache/HashLockInfoCacheTypes.h"
#include "plugins/txes/lock_shared/tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Basic traits for a hash lock info.
	struct BasicHashLockInfoTestTraits : public cache::HashLockInfoCacheDescriptor {
		using HistoryType = cache::HashLockInfoCacheDescriptor::ValueType;
		using LockInfoType = HistoryType::ValueType;
		using ValueType = LockInfoType;

		static constexpr auto ToKey = cache::HashLockInfoCacheDescriptor::GetKeyFromValue;

		/// Creates a hash lock info with given \a height.
		static ValueType CreateLockInfo(Height height);

		/// Creates a random hash lock info.
		static ValueType CreateLockInfo();

		/// Sets the \a key of the \a lockInfo.
		static void SetKey(ValueType& lockInfo, const KeyType& key);

		/// Asserts that the hash lock infos \a lhs and \a rhs are equal.
		static void AssertEqual(const ValueType& lhs, const ValueType& rhs);
	};

	/// Cache factory for creating a catapult cache containing lock hash cache.
	using HashLockInfoCacheFactory = LockInfoCacheFactory<cache::HashLockInfoCacheDescriptor, cache::HashLockInfoCacheStorage>;
}}
