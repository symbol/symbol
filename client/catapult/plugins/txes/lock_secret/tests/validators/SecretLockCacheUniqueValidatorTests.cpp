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
#include "tests/test/SecretLockNotificationsTestUtils.h"

namespace catapult { namespace validators {

#define TEST_CLASS SecretLockCacheUniqueValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(SecretLockCacheUnique, std::unordered_set<Height, utils::BaseValueHasher<Height>>())

	namespace {
		struct SecretCacheTraits {
		public:
			using DescriptorType = test::BasicSecretLockInfoTestTraits;
			using NotificationType = model::SecretLockNotification;
			using NotificationBuilder = test::SecretLockNotificationBuilder;
			using CacheFactory = test::SecretLockInfoCacheFactory;

			static constexpr auto Failure = Failure_LockSecret_Hash_Already_Exists;

			static auto CreateValidator() {
				return CreateSecretLockCacheUniqueValidator(std::unordered_set<Height, utils::BaseValueHasher<Height>>());
			}
		};
	}

	DEFINE_CACHE_UNIQUE_TESTS(SecretCacheTraits)

	TEST(TEST_CLASS, SuccessWhenHashIsInCacheAndActiveAtSkipHeight) {
		// Arrange: configure the validator to skip validation at height 10
		struct CustomSecretCacheTraits : public SecretCacheTraits {
		public:
			static auto CreateValidator() {
				return CreateSecretLockCacheUniqueValidator(std::unordered_set<Height, utils::BaseValueHasher<Height>>{
					Height(10)
				});
			}
		};

		// Act + Assert: notification height is 10
		LockCacheUniqueValidatorTests<CustomSecretCacheTraits>::RunNotEmptyCacheTest(ValidationResult::Success, Height(11), Height(10));
	}
}}
