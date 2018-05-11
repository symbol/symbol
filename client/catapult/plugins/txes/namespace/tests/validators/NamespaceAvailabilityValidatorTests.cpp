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
#include "src/cache/NamespaceCache.h"
#include "src/model/NamespaceConstants.h"
#include "src/model/NamespaceLifetimeConstraints.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define ROOT_TEST_CLASS RootNamespaceAvailabilityValidatorTests
#define CHILD_TEST_CLASS ChildNamespaceAvailabilityValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(RootNamespaceAvailability, model::NamespaceLifetimeConstraints(BlockDuration(), BlockDuration(), 0))
	DEFINE_COMMON_VALIDATOR_TESTS(ChildNamespaceAvailability,)

	namespace {
		constexpr BlockDuration Max_Duration(105);
		constexpr BlockDuration Default_Duration(10);
		constexpr BlockDuration Grace_Period_Duration(20);
		constexpr uint32_t Max_Rollback_Blocks(5);

		template<typename TSeedCacheFunc>
		auto CreateAndSeedCache(TSeedCacheFunc seedCache) {
			auto cache = test::NamespaceCacheFactory::Create();
			{
				auto cacheDelta = cache.createDelta();
				auto& namespaceCacheDelta = cacheDelta.sub<cache::NamespaceCache>();
				seedCache(namespaceCacheDelta);
				cache.commit(Height());
			}
			return cache;
		}

		template<typename TSeedCacheFunc>
		void RunRootTest(
				ValidationResult expectedResult,
				const model::RootNamespaceNotification& notification,
				Height height,
				TSeedCacheFunc seedCache) {
			// Arrange: seed the cache
			auto cache = CreateAndSeedCache(seedCache);

			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(height, readOnlyCache);

			model::NamespaceLifetimeConstraints constraints(Max_Duration, Grace_Period_Duration, Max_Rollback_Blocks);
			auto pValidator = CreateRootNamespaceAvailabilityValidator(constraints);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "height " << height << ", duration " << notification.Duration;
		}

		template<typename TSeedCacheFunc>
		void RunChildTest(
				ValidationResult expectedResult,
				const model::ChildNamespaceNotification& notification,
				Height height,
				TSeedCacheFunc seedCache) {
			// Arrange: seed the cache
			auto cache = CreateAndSeedCache(seedCache);

			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(height, readOnlyCache);

			auto pValidator = CreateChildNamespaceAvailabilityValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "height " << height;
		}

		state::RootNamespace CreateRootNamespace(NamespaceId id, const state::NamespaceLifetime& lifetime) {
			return state::RootNamespace(id, test::CreateRandomOwner(), lifetime);
		}
	}

	namespace {
		void SeedCacheWithRoot25(cache::NamespaceCacheDelta& namespaceCacheDelta) {
			// Arrange: create a cache with { 25 }
			namespaceCacheDelta.insert(CreateRootNamespace(NamespaceId(25), test::CreateLifetime(10, 20)));

			// Sanity:
			test::AssertCacheContents(namespaceCacheDelta, { 25 });
		}

		auto SeedCacheWithRoot25Signer(const Key& signer) {
			return [&signer](auto& namespaceCacheDelta) {
				// Arrange: create a cache with { 25 }
				namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), signer, test::CreateLifetime(10, 20)));

				// Sanity:
				test::AssertCacheContents(namespaceCacheDelta, { 25 });
			};
		}
	}

	// region root - eternal namespace duration check

	TEST(ROOT_TEST_CLASS, CanAddRootNamespaceWithEternalDurationInNemesis) {
		// Act: try to create a root with an eternal duration
		auto notification = model::RootNamespaceNotification(Key(), NamespaceId(26), Eternal_Artifact_Duration);
		RunRootTest(ValidationResult::Success, notification, Height(1), SeedCacheWithRoot25);
	}

	TEST(ROOT_TEST_CLASS, CannotAddRootNamespaceWithEternalDurationAfterNemesis) {
		// Act: try to create a root with an eternal duration
		auto notification = model::RootNamespaceNotification(Key(), NamespaceId(26), Eternal_Artifact_Duration);
		RunRootTest(Failure_Namespace_Eternal_After_Nemesis_Block, notification, Height(15), SeedCacheWithRoot25);
	}

	TEST(ROOT_TEST_CLASS, CannotRenewNonEternalRootNamespaceWithEternalDurationAfterNemesis) {
		// Act: try to renew a root with an eternal duration
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = model::RootNamespaceNotification(signer, NamespaceId(25), Eternal_Artifact_Duration);
		RunRootTest(Failure_Namespace_Eternal_After_Nemesis_Block, notification, Height(15), SeedCacheWithRoot25Signer(signer));
	}

	// endregion

	// region root - non-eternal (new)

	TEST(ROOT_TEST_CLASS, CanAddNewRootNamespaceWithNonEternalDuration) {
		// Arrange:
		for (auto height : { Height(1), Height(15) }) {
			// Act: try to create a root with a non-eternal duration
			auto notification = model::RootNamespaceNotification(Key(), NamespaceId(26), Default_Duration);
			RunRootTest(ValidationResult::Success, notification, height, SeedCacheWithRoot25);
		}
	}

	// endregion

	// region root - renew (owner grace period not expired)

	TEST(ROOT_TEST_CLASS, CanRenewRootNamespaceWithSameOwnerBeforeGracePeriodExpiration) {
		// Arrange: namespace is deactivated at height 20 and grace period is 25, so it is available starting at 45
		for (auto height : { Height(15), Height(20), Height(44) }) {
			// Act: try to renew the owner of a root that has not yet exceeded its grace period
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::RootNamespaceNotification(signer, NamespaceId(25), Default_Duration);
			RunRootTest(ValidationResult::Success, notification, height, SeedCacheWithRoot25Signer(signer));
		}
	}

	TEST(ROOT_TEST_CLASS, CannotChangeRootNamespaceOwnerBeforeGracePeriodExpiration) {
		// Arrange: namespace is deactivated at height 20 and grace period is 25, so it is available starting at 45
		for (auto height : { Height(15), Height(20), Height(44) }) {
			// Act: try to change the owner of a root that has not yet exceeded its grace period
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::RootNamespaceNotification(signer, NamespaceId(25), Default_Duration);
			RunRootTest(Failure_Namespace_Owner_Conflict, notification, height, SeedCacheWithRoot25);
		}
	}

	// endregion

	// region root - renew (owner grace period expired)

	TEST(ROOT_TEST_CLASS, CanRenewRootNamespaceWithSameOwnerAfterGracePeriodExpiration) {
		// Arrange: namespace is deactivated at height 20 and grace period is 25, so it is available starting at 45
		for (auto height : { Height(45), Height(100) }) {
			// Act: try to renew the owner of a root that has expired and exceeded its grace period
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::RootNamespaceNotification(signer, NamespaceId(25), Default_Duration);
			RunRootTest(ValidationResult::Success, notification, height, SeedCacheWithRoot25Signer(signer));
		}
	}

	TEST(ROOT_TEST_CLASS, CanChangeRootNamespaceOwnerAfterGracePeriodExpiration) {
		// Arrange: namespace is deactivated at height 20 and grace period is 25, so it is available starting at 45
		for (auto height : { Height(45), Height(100) }) {
			// Act: try to change the owner of a root that has expired and exceeded its grace period
			auto notification = model::RootNamespaceNotification(Key(), NamespaceId(25), Default_Duration);
			RunRootTest(ValidationResult::Success, notification, height, SeedCacheWithRoot25);
		}
	}

	// endregion

	// region root - renew duration

	namespace {
		void AssertCannotChangeDuration(Height height, const state::NamespaceLifetime& lifetime, BlockDuration duration) {
			// Act: try to extend a root that is already in the cache
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::RootNamespaceNotification(signer, NamespaceId(25), duration);
			RunRootTest(
					Failure_Namespace_Invalid_Duration,
					notification,
					height,
					[&signer, &lifetime](auto& namespaceCacheDelta) {
						// Arrange: create a cache with { 25 }
						namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), signer, lifetime));

						// Sanity:
						test::AssertCacheContents(namespaceCacheDelta, { 25 });
					});
		}
	}

	TEST(ROOT_TEST_CLASS, CannotRenewNonEternalRootNamespaceWithEternalDurationInNemesis) {
		// Assert: extend a non-eternal namespace as eternal
		AssertCannotChangeDuration(Height(1), test::CreateLifetime(10, 20), Eternal_Artifact_Duration);
	}

	TEST(ROOT_TEST_CLASS, CannotRenewRootNamespaceWithEternalDuration) {
		// Assert: "extend" an external namespace
		AssertCannotChangeDuration(Height(1), test::CreateLifetime(10, 0xFFFF'FFFF'FFFF'FFFF), Eternal_Artifact_Duration);
		AssertCannotChangeDuration(Height(100), test::CreateLifetime(10, 0xFFFF'FFFF'FFFF'FFFF), BlockDuration(2));
	}

	TEST(ROOT_TEST_CLASS, CannotRenewRootNamespaceWithDurationTooLarge) {
		// Arrange: max duration is 120 [Max_Duration(105) + Grace_Period_Duration(20) + height(15) - lifetime.End(20)]
		for (auto duration : { BlockDuration(121), BlockDuration(200) })
			AssertCannotChangeDuration(Height(15), test::CreateLifetime(10, 20), duration);
	}

	TEST(ROOT_TEST_CLASS, CanRenewRootNamespaceWithAcceptableDurations) {
		// Arrange: max duration is 120 [Max_Duration(105) + Grace_Period_Duration(20) + height(15) - lifetime.End(20)]
		for (auto duration : { BlockDuration(20), BlockDuration(75), BlockDuration(120) }) {
			// Act: try to renew a root
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::RootNamespaceNotification(signer, NamespaceId(25), duration);
			RunRootTest(ValidationResult::Success, notification, Height(15), SeedCacheWithRoot25Signer(signer));
		}
	}

	// endregion

	namespace {
		auto SeedCacheWithRoot25TreeSigner(const Key& signer) {
			return [&signer](auto& namespaceCacheDelta) {
				// Arrange: create a cache with { 25 }, { 25, 36 } and { 25, 36, 49 }
				namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), signer, test::CreateLifetime(10, 20)));
				namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 36 })));
				namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 36, 49 })));

				// Sanity:
				test::AssertCacheContents(namespaceCacheDelta, { 25, 36, 49 });
			};
		}
	}

	// region child - existence

	TEST(CHILD_TEST_CLASS, CanAddChildNamespaceThatDoesNotExistToRootParent) {
		// Arrange:
		for (auto height : { Height(10), Height(15), Height(19) }) {
			// Act: try to create a child that is not in the cache (parent is root 25)
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::ChildNamespaceNotification(signer, NamespaceId(38), NamespaceId(25));
			RunChildTest(ValidationResult::Success, notification, height, SeedCacheWithRoot25TreeSigner(signer));
		}
	}

	TEST(CHILD_TEST_CLASS, CanAddChildNamespaceThatDoesNotExistToNonRootParent) {
		// Arrange:
		for (auto height : { Height(10), Height(15), Height(19) }) {
			// Act: try to create a child that is not in the cache (parent is non-root 36)
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::ChildNamespaceNotification(signer, NamespaceId(50), NamespaceId(36));
			RunChildTest(ValidationResult::Success, notification, height, SeedCacheWithRoot25TreeSigner(signer));
		}
	}

	TEST(CHILD_TEST_CLASS, CannotAddChildNamespaceThatAlreadyExists) {
		// Act: try to create a child that is already in the cache
		for (auto height : { Height(15), Height(19) }) {
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::ChildNamespaceNotification(signer, NamespaceId(36), NamespaceId(25));
			RunChildTest(Failure_Namespace_Already_Exists, notification, height, SeedCacheWithRoot25TreeSigner(signer));
		};
	}

	// endregion

	// region child - parent

	TEST(CHILD_TEST_CLASS, CannotAddChildNamespaceThatHasUnknownParent) {
		// Act: try to create a child with an unknown (root) parent
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = model::ChildNamespaceNotification(signer, NamespaceId(38), NamespaceId(26));
		RunChildTest(Failure_Namespace_Parent_Unknown, notification, Height(15), SeedCacheWithRoot25TreeSigner(signer));
	}

	TEST(CHILD_TEST_CLASS, CannotAddChildNamespaceToParentWithMaxNamespaceDepth) {
		// Act: try to create a child that is too deep
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = model::ChildNamespaceNotification(signer, NamespaceId(64), NamespaceId(49));
		RunChildTest(Failure_Namespace_Too_Deep, notification, Height(15), SeedCacheWithRoot25TreeSigner(signer));
	}

	// endregion

	// region child - root

	TEST(CHILD_TEST_CLASS, CannotAddChildNamespaceToExpiredRoot) {
		// Arrange:
		for (auto height : { Height(20), Height(25), Height(100) }) {
			// Act: try to create a child attached to a root that has expired (root namespace expires at height 20)
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::ChildNamespaceNotification(signer, NamespaceId(50), NamespaceId(36));
			RunChildTest(Failure_Namespace_Expired, notification, height, SeedCacheWithRoot25TreeSigner(signer));
		}
	}

	TEST(CHILD_TEST_CLASS, CannotAddChildNamespaceToRootWithConflictingOwner) {
		// Arrange:
		for (auto height : { Height(10), Height(15), Height(19) }) {
			// Act: try to create a child attached to a root with a different owner
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = model::ChildNamespaceNotification(signer, NamespaceId(50), NamespaceId(36));
			auto rootSigner = test::CreateRandomOwner();
			RunChildTest(Failure_Namespace_Owner_Conflict, notification, height, SeedCacheWithRoot25TreeSigner(rootSigner));
		}
	}

	// endregion
}}
