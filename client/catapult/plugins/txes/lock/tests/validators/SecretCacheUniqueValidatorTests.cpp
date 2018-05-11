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

#include "src/validators/Validators.h"
#include "tests/test/CacheUniqueTestUtils.h"
#include "tests/test/LockNotificationsTestUtils.h"

namespace catapult { namespace validators {

#define TEST_CLASS SecretCacheUniqueValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(SecretCacheUnique,)

	namespace {
		struct SecretCacheTraits {
		public:
			using DescriptorType = test::BasicSecretLockInfoTestTraits;
			using NotificationType = model::SecretLockNotification;
			using NotificationBuilder = test::SecretLockNotificationBuilder;
			using CacheFactory = test::SecretLockInfoCacheFactory;

			static constexpr auto Failure = Failure_Lock_Hash_Exists;

			static auto CreateValidator() {
				return CreateSecretCacheUniqueValidator();
			}
		};
	}

	DEFINE_CACHE_UNIQUE_TESTS(SecretCacheTraits)
}}
