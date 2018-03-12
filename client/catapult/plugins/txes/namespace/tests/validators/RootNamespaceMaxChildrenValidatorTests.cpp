#include "src/validators/Validators.h"
#include "src/cache/NamespaceCache.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS RootNamespaceMaxChildrenValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(RootNamespaceMaxChildren, 123)

	namespace {
		auto CreateAndSeedCache() {
			auto cache = test::NamespaceCacheFactory::Create();
			{
				auto cacheDelta = cache.createDelta();
				auto& namespaceCacheDelta = cacheDelta.sub<cache::NamespaceCache>();
				auto rootOwner = test::GenerateRandomData<Key_Size>();

				namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), rootOwner, test::CreateLifetime(10, 20)));
				namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 36 })));
				namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 36, 49 })));
				namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 37 })));

				// Sanity:
				test::AssertCacheContents(namespaceCacheDelta, { 25, 36, 49, 37 });

				cache.commit(Height());
			}
			return cache;
		}

		void RunTest(ValidationResult expectedResult, const model::ChildNamespaceNotification& notification, uint16_t maxChildren) {
			// Arrange: seed the cache
			auto cache = CreateAndSeedCache();

			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(), readOnlyCache);

			auto pValidator = CreateRootNamespaceMaxChildrenValidator(maxChildren);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "maxChildren " << maxChildren;
		}
	}

	TEST(TEST_CLASS, FailureIfMaxChildrenIsExceeded) {
		// Act: root with id 25 has 3 children
		auto notification = model::ChildNamespaceNotification(Key(), NamespaceId(26), NamespaceId(25));
		RunTest(Failure_Namespace_Max_Children_Exceeded, notification, 1);
		RunTest(Failure_Namespace_Max_Children_Exceeded, notification, 2);
		RunTest(Failure_Namespace_Max_Children_Exceeded, notification, 3);
	}

	TEST(TEST_CLASS, SuccessIfMaxChildrenIsNotExceeded) {
		// Act: root with id 25 has 3 children
		auto notification = model::ChildNamespaceNotification(Key(), NamespaceId(26), NamespaceId(25));
		RunTest(ValidationResult::Success, notification, 4);
		RunTest(ValidationResult::Success, notification, 5);
		RunTest(ValidationResult::Success, notification, 123);
	}
}}
