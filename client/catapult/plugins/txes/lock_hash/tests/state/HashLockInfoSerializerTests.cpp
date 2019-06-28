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

#include "src/state/HashLockInfoSerializer.h"
#include "plugins/txes/lock_shared/tests/state/LockInfoSerializerTests.h"
#include "tests/test/HashLockInfoCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS HashLockInfoSerializerTests

	namespace {
		// region PackedHashLockInfo

#pragma pack(push, 1)

		struct PackedHashLockInfo : public PackedLockInfo {
		public:
			explicit PackedHashLockInfo(const HashLockInfo& hashLockInfo)
					: PackedLockInfo(hashLockInfo)
					, Hash(hashLockInfo.Hash)
			{}

		public:
			Hash256 Hash;
		};

#pragma pack(pop)

		// endregion

		struct HashLockInfoStorageTraits : public test::BasicHashLockInfoTestTraits {
			using PackedValueType = PackedHashLockInfo;
			using SerializerType = HashLockInfoSerializer;
		};
	}

	DEFINE_LOCK_INFO_SERIALIZER_TESTS(HashLockInfoStorageTraits)
}}
