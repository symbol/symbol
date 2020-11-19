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

#include "src/validators/Validators.h"
#include "plugins/txes/lock_shared/tests/validators/LockCacheUniqueValidatorTests.h"
#include "tests/test/HashLockNotificationsTestUtils.h"

namespace catapult { namespace validators {

#define TEST_CLASS HashLockCacheUniqueValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(HashLockCacheUnique,)

	namespace {
		struct HashCacheTraits {
		public:
			using DescriptorType = test::BasicHashLockInfoTestTraits;
			using NotificationType = model::HashLockNotification;
			using NotificationBuilder = test::HashLockNotificationBuilder;
			using CacheFactory = test::HashLockInfoCacheFactory;

			static constexpr auto Failure = Failure_LockHash_Hash_Already_Exists;

			static auto CreateValidator() {
				return CreateHashLockCacheUniqueValidator();
			}
		};
	}

	DEFINE_CACHE_UNIQUE_TESTS(HashCacheTraits)
}}
