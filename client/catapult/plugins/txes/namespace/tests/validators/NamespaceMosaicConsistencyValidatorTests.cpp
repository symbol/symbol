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
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS NamespaceMosaicConsistencyValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(NamespaceMosaicConsistency,)

	namespace {
		model::MosaicDefinitionNotification CreateNotification(const Key& signer, NamespaceId parentId, MosaicId id) {
			return model::MosaicDefinitionNotification(signer, parentId, id, model::MosaicProperties::FromValues({}));
		}

		template<typename TSeedCacheFunc>
		void RunTest(
				ValidationResult expectedResult,
				const model::MosaicDefinitionNotification& notification,
				Height height,
				TSeedCacheFunc seedCache) {
			// Arrange: seed the cache
			auto cache = test::NamespaceCacheFactory::Create();
			{
				auto cacheDelta = cache.createDelta();
				auto& namespaceCacheDelta = cacheDelta.sub<cache::NamespaceCache>();
				seedCache(namespaceCacheDelta);
				cache.commit(Height());
			}

			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(height, readOnlyCache);

			auto pValidator = CreateNamespaceMosaicConsistencyValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "height " << height << ", id " << notification.MosaicId;
		}
	}

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

	TEST(TEST_CLASS, CanAddMosaicWithKnownValidParent) {
		// Arrange:
		for (auto height : { Height(10), Height(15), Height(19) }) {
			// Act: try to create a mosaic that is not in the cache (parent is root 25)
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = CreateNotification(signer, NamespaceId(25), MosaicId(38));
			RunTest(ValidationResult::Success, notification, height, SeedCacheWithRoot25TreeSigner(signer));
		}
	}

	TEST(TEST_CLASS, CannotAddMosaicThatHasUnknownParent) {
		// Act: try to create a mosaic with an unknown (root) parent
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, NamespaceId(26), MosaicId(38));
		RunTest(Failure_Namespace_Parent_Unknown, notification, Height(15), SeedCacheWithRoot25TreeSigner(signer));
	}

	TEST(TEST_CLASS, CannotAddChildNamespaceToExpiredRoot) {
		// Arrange:
		for (auto height : { Height(20), Height(25), Height(100) }) {
			// Act: try to create a mosaic attached to a root that has expired (root namespace expires at height 20)
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = CreateNotification(signer, NamespaceId(36), MosaicId(50));
			RunTest(Failure_Namespace_Expired, notification, height, SeedCacheWithRoot25TreeSigner(signer));
		}
	}

	TEST(TEST_CLASS, CannotAddChildNamespaceToRootWithConflictingOwner) {
		// Arrange:
		for (auto height : { Height(10), Height(15), Height(19) }) {
			// Act: try to create a mosaic attached to a root with a different owner
			auto signer = test::GenerateRandomData<Key_Size>();
			auto notification = CreateNotification(signer, NamespaceId(36), MosaicId(50));
			auto rootSigner = test::CreateRandomOwner();
			RunTest(Failure_Namespace_Owner_Conflict, notification, height, SeedCacheWithRoot25TreeSigner(rootSigner));
		}
	}
}}
