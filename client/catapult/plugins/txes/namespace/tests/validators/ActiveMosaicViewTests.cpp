#include "src/validators/Validators.h"
#include "src/validators/ActiveMosaicView.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS ActiveMosaicViewTests

	namespace {
		void SeedCacheWithRoot25TreeSigner(cache::NamespaceCacheDelta& namespaceCacheDelta, const Key& signer) {
			// Arrange: create a cache with { 25 } and { 25, 36 }
			namespaceCacheDelta.insert(state::RootNamespace(NamespaceId(25), signer, test::CreateLifetime(10, 123)));
			namespaceCacheDelta.insert(state::Namespace(test::CreatePath({ 25, 36 })));

			// Sanity:
			test::AssertCacheContents(namespaceCacheDelta, { 25, 36 });
		}

		struct TryGetTraits {
			static validators::ValidationResult TryGet(const ActiveMosaicView& view, MosaicId id, Height height) {
				const state::MosaicEntry* pEntry;
				return view.tryGet(id, height, &pEntry);
			}
		};

		struct TryGetWithOwnerTraits {
			static validators::ValidationResult TryGet(const ActiveMosaicView& view, MosaicId id, Height height) {
				const state::MosaicEntry* pEntry;
				return view.tryGet(id, height, Key(), &pEntry);
			}
		};
	}

#define TRY_GET_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TryGetTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_WithOwner) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TryGetWithOwnerTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region tryGet

	// region active checks

	TRY_GET_BASED_TEST(CannotGetUnknownMosaic) {
		// Arrange:
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto delta = cache.createDelta();
		auto readOnlyCache = delta.toReadOnly();
		auto view = ActiveMosaicView(readOnlyCache);

		// Act:
		auto result = TTraits::TryGet(view, MosaicId(123), Height(100));

		// Assert:
		EXPECT_EQ(Failure_Mosaic_Expired, result);
	}

	TRY_GET_BASED_TEST(CannotGetInactiveMosaic) {
		// Arrange:
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto delta = cache.createDelta();
		test::AddMosaic(delta, MosaicId(123), Height(50), ArtifactDuration(100), Amount());
		cache.commit(Height());

		auto readOnlyCache = delta.toReadOnly();
		auto view = ActiveMosaicView(readOnlyCache);

		// Act: mosaic expires at 50 + 100
		auto result = TTraits::TryGet(view, MosaicId(123), Height(200));

		// Assert:
		EXPECT_EQ(Failure_Mosaic_Expired, result);
	}

	TRY_GET_BASED_TEST(CannotGetMosaicWithUnknownNamespace) {
		// Arrange:
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto delta = cache.createDelta();
		test::AddEternalMosaic(delta, NamespaceId(36), MosaicId(123), Height(50));
		cache.commit(Height());

		auto readOnlyCache = delta.toReadOnly();
		auto view = ActiveMosaicView(readOnlyCache);

		// Act:
		auto result = TTraits::TryGet(view, MosaicId(123), Height(100));

		// Assert:
		EXPECT_EQ(Failure_Namespace_Expired, result);
	}

	TRY_GET_BASED_TEST(CannotGetMosaicWithInactiveNamespace) {
		// Arrange:
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto delta = cache.createDelta();
		test::AddEternalMosaic(delta, NamespaceId(36), MosaicId(123), Height(50));
		SeedCacheWithRoot25TreeSigner(delta.sub<cache::NamespaceCache>(), test::GenerateRandomData<Key_Size>());
		cache.commit(Height());

		auto readOnlyCache = delta.toReadOnly();
		auto view = ActiveMosaicView(readOnlyCache);

		// Act: namespace expires at 123
		auto result = TTraits::TryGet(view, MosaicId(123), Height(150));

		// Assert:
		EXPECT_EQ(Failure_Namespace_Expired, result);
	}

	// endregion

	// region owner checks

	template<typename TAction>
	void RunTestWithActiveMosaic(const Key& owner, TAction action) {
		// Arrange:
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto delta = cache.createDelta();
		test::AddEternalMosaic(delta, NamespaceId(36), MosaicId(123), Height(50), owner);
		SeedCacheWithRoot25TreeSigner(delta.sub<cache::NamespaceCache>(), owner);
		cache.commit(Height());

		auto readOnlyCache = delta.toReadOnly();
		auto view = ActiveMosaicView(readOnlyCache);

		// Act + Assert:
		action(view);
	}

	TEST(TEST_CLASS, CanGetActiveMosaicWithoutOwnerCheck) {
		// Arrange:
		RunTestWithActiveMosaic(test::GenerateRandomData<Key_Size>(), [](const auto& view) {
			// Act: namespace expires at 123
			const state::MosaicEntry* pEntry;
			auto result = view.tryGet(MosaicId(123), Height(100), &pEntry);

			// Assert:
			EXPECT_EQ(ValidationResult::Success, result);
			EXPECT_EQ(MosaicId(123), pEntry->mosaicId());
		});
	}

	TEST(TEST_CLASS, CannotGetActiveMosaicWithWrongOwner) {
		// Arrange:
		RunTestWithActiveMosaic(test::GenerateRandomData<Key_Size>(), [](const auto& view) {
			// Act: namespace expires at 123
			const state::MosaicEntry* pEntry;
			auto result = view.tryGet(MosaicId(123), Height(100), test::GenerateRandomData<Key_Size>(), &pEntry);

			// Assert:
			EXPECT_EQ(Failure_Mosaic_Owner_Conflict, result);
		});
	}

	TEST(TEST_CLASS, CanGetActiveMosaicWithCorrectOwner) {
		// Arrange:
		const auto& owner = test::GenerateRandomData<Key_Size>();
		RunTestWithActiveMosaic(owner, [&owner](const auto& view) {
			// Act: namespace expires at 123
			const state::MosaicEntry* pEntry;
			auto result = view.tryGet(MosaicId(123), Height(100), owner, &pEntry);

			// Assert:
			EXPECT_EQ(ValidationResult::Success, result);
			EXPECT_EQ(MosaicId(123), pEntry->mosaicId());
		});
	}

	// endregion

	// endregion
}}
