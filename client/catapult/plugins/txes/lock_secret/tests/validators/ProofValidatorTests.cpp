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
#include "tests/test/SecretLockInfoCacheTestUtils.h"
#include "tests/test/SecretLockNotificationsTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
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

		auto CreateDefaultLockInfo(const Hash256& hash, const UnresolvedAddress& recipient) {
			auto lockInfo = test::BasicSecretLockInfoTestTraits::CreateLockInfo(Expiration_Height);
			lockInfo.HashAlgorithm = model::LockHashAlgorithm::Op_Sha3_256;
			lockInfo.Secret = hash;
			lockInfo.RecipientAddress = test::CreateResolverContextXor().resolve(recipient);
			lockInfo.CompositeHash = model::CalculateSecretLockInfoHash(lockInfo.Secret, lockInfo.RecipientAddress);
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
			for (auto i = 0u; i < 10; ++i) {
				auto lockInfo = CreateDefaultLockInfo(test::GenerateRandomByteArray<Hash256>(), test::GenerateRandomUnresolvedAddress());
				AddToCache(cache, lockInfo);
			}

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

	TEST(TEST_CLASS, FailureWhenSecretIsNotInCache) {
		// Arrange: notification has random secret
		auto notificationBuilder = CreateNotificationBuilder();
		auto cache = CreateDefaultSeededCache();
		auto pValidator = CreateProofValidator();

		// Act:
		auto result = RunValidator(cache, *pValidator, notificationBuilder);

		// Assert:
		EXPECT_EQ(Failure_LockSecret_Unknown_Composite_Key, result);
	}

	namespace {
		void AssertValidationResult(
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

		void AssertValidationResult(ValidationResult expectedResult, const test::ProofNotificationBuilder& notificationBuilder) {
			auto lockInfo = CreateDefaultLockInfo(notificationBuilder.hash(), notificationBuilder.recipient());
			AssertValidationResult(expectedResult, lockInfo, notificationBuilder);
		}
	}

	TEST(TEST_CLASS, FailureWhenSecretIsNotActive) {
		// Arrange:
		auto notificationBuilder = CreateNotificationBuilder();
		notificationBuilder.setHeight(Expiration_Height);

		// Assert:
		AssertValidationResult(Failure_LockSecret_Inactive_Secret, notificationBuilder);
	}

	TEST(TEST_CLASS, FailureWhenHashAlgorithmDoesNotMatch) {
		// Arrange:
		auto notificationBuilder = CreateNotificationBuilder();
		notificationBuilder.setAlgorithm(model::LockHashAlgorithm::Op_Hash_256);

		// Assert:
		AssertValidationResult(Failure_LockSecret_Hash_Algorithm_Mismatch, notificationBuilder);
	}

	TEST(TEST_CLASS, FailureWhenLockHasAlreadyBeenUsed) {
		// Arrange:
		auto notificationBuilder = CreateNotificationBuilder();
		auto lockInfo = CreateDefaultLockInfo(notificationBuilder.hash(), notificationBuilder.recipient());
		lockInfo.Status = state::LockStatus::Used;

		// Assert:
		AssertValidationResult(Failure_LockSecret_Inactive_Secret, lockInfo, notificationBuilder);
	}

	TEST(TEST_CLASS, SuccessWhenValidSecretIsInCache) {
		// Arrange:
		auto notificationBuilder = CreateNotificationBuilder();

		// Assert:
		AssertValidationResult(ValidationResult::Success, notificationBuilder);
	}
}}
