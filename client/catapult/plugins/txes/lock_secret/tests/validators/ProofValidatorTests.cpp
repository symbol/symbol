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
#include "tests/test/SecretLockInfoCacheTestUtils.h"
#include "tests/test/SecretLockNotificationsTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"

namespace catapult { namespace validators {

#define TEST_CLASS ProofValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(Proof,)

	namespace {
		constexpr Height Expiration_Height(321);
		constexpr Height Default_Height(320);

		auto CreateNotificationBuilder() {
			return test::ProofNotificationBuilder(Default_Height);
		}

		auto CreateDefaultLockInfo(const Hash256& hash) {
			auto lockInfo = test::BasicSecretLockInfoTestTraits::CreateLockInfo(Expiration_Height);
			lockInfo.HashAlgorithm = model::LockHashAlgorithm::Op_Sha3_256;
			lockInfo.Secret = hash;
			return lockInfo;
		}

		void AddToCache(cache::CatapultCache& cache, const state::SecretLockInfo& lockInfo) {
			auto cacheDelta = cache.createDelta();
			auto& lockInfoCacheDelta = cacheDelta.sub<cache::SecretLockInfoCache>();
			lockInfoCacheDelta.insert(lockInfo);
			cache.commit(Height());
		}

		auto CreateDefaultSeededCache() {
			auto cache = test::SecretLockInfoCacheFactory::Create();
			for (auto i = 0u; i < 10; ++i)
				AddToCache(cache, CreateDefaultLockInfo(test::GenerateRandomData<Hash256_Size>()));

			return cache;
		}

		template<typename TValidator>
		auto RunValidator(
				cache::CatapultCache& cache,
				const TValidator& validator,
				const test::ProofNotificationBuilder& notificationBuilder) {
			return test::ValidateNotification(
					validator,
					notificationBuilder.notification(),
					cache,
					notificationBuilder.notificationHeight());
		}
	}

	TEST(TEST_CLASS, FailureIfSecretIsNotInCache) {
		// Arrange: notification has random secret
		auto notificationBuilder = CreateNotificationBuilder();
		auto cache = CreateDefaultSeededCache();
		auto pValidator = CreateProofValidator();

		// Act:
		auto result = RunValidator(cache, *pValidator, notificationBuilder);

		// Assert:
		EXPECT_EQ(Failure_LockSecret_Unknown_Secret, result);
	}

	namespace {
		void AssertValidatorResult(
				ValidationResult expectedResult,
				const state::SecretLockInfo& lockInfo,
				const test::ProofNotificationBuilder& notificationBuilder) {
			auto cache = CreateDefaultSeededCache();
			AddToCache(cache, lockInfo);
			auto pValidator = CreateProofValidator();

			// Act:
			auto result = RunValidator(cache, *pValidator, notificationBuilder);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		void AssertValidatorResult(ValidationResult expectedResult, const test::ProofNotificationBuilder& notificationBuilder) {
			AssertValidatorResult(expectedResult, CreateDefaultLockInfo(notificationBuilder.hash()), notificationBuilder);
		}
	}

	TEST(TEST_CLASS, FailureIfSecretIsNotActive) {
		// Arrange:
		auto notificationBuilder = CreateNotificationBuilder();
		notificationBuilder.setHeight(Expiration_Height);

		// Assert:
		AssertValidatorResult(Failure_LockSecret_Inactive_Secret, notificationBuilder);
	}

	TEST(TEST_CLASS, FailureIfHashAlgorithmDoesNotMatch) {
		// Arrange:
		auto notificationBuilder = CreateNotificationBuilder();
		notificationBuilder.setAlgorithm(model::LockHashAlgorithm::Op_Keccak_256);

		// Assert:
		AssertValidatorResult(Failure_LockSecret_Hash_Algorithm_Mismatch, notificationBuilder);
	}

	TEST(TEST_CLASS, FailureIfLockHasAlreadyBeenUsed) {
		// Arrange:
		auto notificationBuilder = CreateNotificationBuilder();
		auto lockInfo = CreateDefaultLockInfo(notificationBuilder.hash());
		lockInfo.Status = state::LockStatus::Used;

		// Assert:
		AssertValidatorResult(Failure_LockSecret_Inactive_Secret, lockInfo, notificationBuilder);
	}

	TEST(TEST_CLASS, SuccessIfValidSecretIsInCache) {
		// Arrange:
		auto notificationBuilder = CreateNotificationBuilder();

		// Assert:
		AssertValidatorResult(ValidationResult::Success, notificationBuilder);
	}
}}
