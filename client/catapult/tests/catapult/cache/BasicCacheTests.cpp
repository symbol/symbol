#include "catapult/cache/BasicCache.h"
#include "catapult/deltaset/BaseSet.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/deltaset/OrderedSet.h"
#include "tests/TestHarness.h"
#include <unordered_map>

namespace catapult { namespace cache {

#define TEST_CLASS BasicCacheTests

	namespace {
		// define a base set for (int, string); since no conversions happen TElementToKeyConverter can be void
		using MapStorageTraits = deltaset::MapStorageTraits<std::unordered_map<int, std::string>, void>;
		using BaseSetType = deltaset::BaseSet<deltaset::MutableTypeTraits<std::string>, MapStorageTraits>;

		using OrderedSetType = deltaset::OrderedSet<deltaset::MutableTypeTraits<std::string>>;

		// create a cache descriptor around BaseSetType with view and delta types that simply capture parameters
		// provide two constructors for each sub view so it can be used with and without options
		template<typename TBaseSet>
		struct TestCacheDescriptor {
			struct CacheViewType {
			public:
				using ReadOnlyView = void;

			public:
				explicit CacheViewType(const TBaseSet& set) : Set(set)
				{}

				explicit CacheViewType(const TBaseSet& set, int tag)
						: Set(set)
						, Tag(tag)
				{}

			public:
				const TBaseSet& Set;
				int Tag;
			};

			struct CacheDeltaType {
			private:
				using DeltaPointerType = std::shared_ptr<typename TBaseSet::DeltaType>;

			public:
				explicit CacheDeltaType(const DeltaPointerType& pDeltaParam) : pDelta(pDeltaParam)
				{}

				explicit CacheDeltaType(const DeltaPointerType& pDeltaParam, int tag)
						: pDelta(pDeltaParam)
						, Tag(tag)
				{}

			public:
				// required by TestCacheOrdered
				auto pruningBoundary() const {
					return deltaset::PruningBoundary<std::string>();
				}

			public:
				DeltaPointerType pDelta;
				int Tag;
			};
		};

		// define the test caches using BasicCache
		using TestCache = BasicCache<TestCacheDescriptor<BaseSetType>, BaseSetType>;
		using TestCacheWithOptions = BasicCache<TestCacheDescriptor<BaseSetType>, BaseSetType, int>;
		using TestCacheOrdered = BasicCache<TestCacheDescriptor<OrderedSetType>, OrderedSetType>;
	}

	// region sub views - no options

	TEST(TEST_CLASS, CanCreateViews) {
		// Arrange:
		TestCache cache;

		// Act:
		auto view1 = cache.createView();
		auto view2 = cache.createView();

		// Assert:
		EXPECT_EQ(&view1.Set, &view2.Set);
	}

	TEST(TEST_CLASS, CanCreateDelta) {
		// Arrange:
		TestCache cache;

		// Act:
		auto delta = cache.createDelta();

		// Assert:
		EXPECT_TRUE(!!delta.pDelta);
	}

	TEST(TEST_CLASS, CanCreateDetachedDeltas) {
		// Arrange:
		TestCache cache;

		// Act:
		auto delta1 = cache.createDetachedDelta();
		auto delta2 = cache.createDetachedDelta();

		// Assert:
		EXPECT_TRUE(!!delta1.pDelta);
		EXPECT_TRUE(!!delta2.pDelta);
		EXPECT_NE(delta1.pDelta, delta2.pDelta);
	}

	// endregion

	// region sub views - with options

	TEST(TEST_CLASS, CanCreateViews_WithOptions) {
		// Arrange:
		TestCacheWithOptions cache(17);

		// Act:
		auto view1 = cache.createView();
		auto view2 = cache.createView();

		// Assert:
		EXPECT_EQ(&view1.Set, &view2.Set);

		EXPECT_EQ(17, view1.Tag);
		EXPECT_EQ(17, view2.Tag);
	}

	TEST(TEST_CLASS, CanCreateDelta_WithOptions) {
		// Arrange:
		TestCacheWithOptions cache(17);

		// Act:
		auto delta = cache.createDelta();

		// Assert:
		EXPECT_TRUE(!!delta.pDelta);

		EXPECT_EQ(17, delta.Tag);
	}

	TEST(TEST_CLASS, CanCreateDetachedDeltas_WithOptions) {
		// Arrange:
		TestCacheWithOptions cache(17);

		// Act:
		auto delta1 = cache.createDetachedDelta();
		auto delta2 = cache.createDetachedDelta();

		// Assert:
		EXPECT_TRUE(!!delta1.pDelta);
		EXPECT_TRUE(!!delta2.pDelta);
		EXPECT_NE(delta1.pDelta, delta2.pDelta);

		EXPECT_EQ(17, delta1.Tag);
		EXPECT_EQ(17, delta2.Tag);
	}

	// endregion

	// region commit

#define COMMIT_TEST(TEST_NAME) \
	template<typename TCache> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TestCache>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Ordered) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TestCacheOrdered>(); } \
	template<typename TCache> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	COMMIT_TEST(CanCommitDelta) {
		// Arrange:
		TCache cache;

		// Act:
		auto delta = cache.createDelta();

		// Assert: no exception
		cache.commit(delta);
	}

	COMMIT_TEST(CannotCommitDetachedDelta) {
		// Arrange:
		TCache cache;

		// Act:
		auto delta = cache.createDetachedDelta();

		// Assert:
		EXPECT_THROW(cache.commit(delta), catapult_runtime_error);
	}

	// endregion
}}
