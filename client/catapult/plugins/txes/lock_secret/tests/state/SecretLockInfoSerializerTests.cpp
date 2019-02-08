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

#include "src/state/SecretLockInfoSerializer.h"
#include "plugins/txes/lock_shared/tests/state/LockInfoSerializerTests.h"
#include "tests/test/SecretLockInfoCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS SecretLockInfoSerializerTests

	namespace {
		// region PackedSecretLockInfo

#pragma pack(push, 1)

		struct PackedSecretLockInfo : public PackedLockInfo {
		public:
			explicit PackedSecretLockInfo(const SecretLockInfo& secretLockInfo)
					: PackedLockInfo(secretLockInfo)
					, HashAlgorithm(secretLockInfo.HashAlgorithm)
					, Secret(secretLockInfo.Secret)
					, Recipient(secretLockInfo.Recipient)
			{}

		public:
			model::LockHashAlgorithm HashAlgorithm;
			Hash256 Secret;
			catapult::Address Recipient;
		};

#pragma pack(pop)

		// endregion

		struct SecretLockInfoStorageTraits : public test::BasicSecretLockInfoTestTraits {
			using PackedValueType = PackedSecretLockInfo;
			using SerializerType = SecretLockInfoSerializer;

			static size_t ValueTypeSize() {
				return sizeof(PackedValueType);
			}
		};
	}

	DEFINE_LOCK_INFO_SERIALIZER_TESTS(SecretLockInfoStorageTraits)
}}
