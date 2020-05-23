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
#include "tests/test/AliasTestUtils.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"

using namespace catapult::model;

namespace catapult { namespace validators {

#define TEST_CLASS RequiredNamespaceValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(RequiredNamespace,)

	namespace {
		constexpr auto Default_Namespace_Id = NamespaceId(123);
		constexpr auto Grace_Period_Duration = BlockDuration(100);

		template<typename TSeedCacheFunc>
		auto CreateAndSeedCache(TSeedCacheFunc seedCache) {
			auto cache = test::NamespaceCacheFactory::Create(Grace_Period_Duration);
			auto cacheDelta = cache.createDelta();
			auto& namespaceCacheDelta = cacheDelta.sub<cache::NamespaceCache>();
			seedCache(namespaceCacheDelta);
			cache.commit(Height());
			return cache;
		}

		template<typename TSeedCacheFunc>
		void RunAvailabilityTest(
				ValidationResult expectedResult,
				const NamespaceRequiredNotification& notification,
				TSeedCacheFunc seedCache) {
			// Arrange:
			auto cache = CreateAndSeedCache(seedCache);
			auto pValidator = CreateRequiredNamespaceValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache, Height(200));

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, FailureWhenNamespaceIsUnknown) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		NamespaceRequiredNotification notification(owner, Default_Namespace_Id);

		// Assert:
		RunAvailabilityTest(Failure_Namespace_Unknown, notification, [](const auto&) {});
	}

	TEST(TEST_CLASS, FailureWhenOwnerDoesNotMatch) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		NamespaceRequiredNotification notification(owner, Default_Namespace_Id);

		// Assert:
		RunAvailabilityTest(Failure_Namespace_Owner_Conflict, notification, [&owner](auto& cache) {
			auto namespaceOwner = owner;
			namespaceOwner[0] ^= 0xFF;
			auto lifetime = test::CreateLifetime(100, 300 + Grace_Period_Duration.unwrap());
			cache.insert(state::RootNamespace(Default_Namespace_Id, namespaceOwner, lifetime));
		});
	}

	TEST(TEST_CLASS, FailureWhenNamespaceExpired) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		NamespaceRequiredNotification notification(owner, Default_Namespace_Id);

		// Assert: notification is at height 200, so limit lifetime to 175 (including grace period)
		RunAvailabilityTest(Failure_Namespace_Expired, notification, [&owner](auto& cache) {
			auto lifetime = test::CreateLifetime(50, 75 + Grace_Period_Duration.unwrap());
			cache.insert(state::RootNamespace(Default_Namespace_Id, owner, lifetime));
		});
	}

	TEST(TEST_CLASS, FailureWhenNamespaceInGracePeriod) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		NamespaceRequiredNotification notification(owner, Default_Namespace_Id);

		// Assert: notification is at height 200, so limit lifetime to 250 (including grace period)
		RunAvailabilityTest(Failure_Namespace_Expired, notification, [&owner](auto& cache) {
			auto lifetime = test::CreateLifetime(100, 150 + Grace_Period_Duration.unwrap());
			cache.insert(state::RootNamespace(Default_Namespace_Id, owner, lifetime));
		});
	}

	TEST(TEST_CLASS, SuccessWhenNamespaceActiveAndOwnerMatches) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		NamespaceRequiredNotification notification(owner, Default_Namespace_Id);

		// Assert:
		RunAvailabilityTest(ValidationResult::Success, notification, [&owner](auto& cache) {
			auto lifetime = test::CreateLifetime(100, 300 + Grace_Period_Duration.unwrap());
			cache.insert(state::RootNamespace(Default_Namespace_Id, owner, lifetime));
		});
	}

	TEST(TEST_CLASS, SuccessWhenNamespaceActiveAndOwnerMatches_UnresolvedAddress) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		NamespaceRequiredNotification notification(test::UnresolveXor(owner), Default_Namespace_Id);

		// Assert:
		RunAvailabilityTest(ValidationResult::Success, notification, [&owner](auto& cache) {
			auto lifetime = test::CreateLifetime(100, 300 + Grace_Period_Duration.unwrap());
			cache.insert(state::RootNamespace(Default_Namespace_Id, owner, lifetime));
		});
	}
}}
