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
		class BaseSetTypeUnorderedExplicit : public deltaset::BaseSet<deltaset::MutableTypeTraits<std::string>, MapStorageTraits> {
		public:
			using IsOrderedSet = std::false_type;

		public:
			explicit BaseSetTypeUnorderedExplicit(const CacheConfiguration&)
			{}
		};

		class OrderedSetType : public deltaset::OrderedSet<deltaset::MutableTypeTraits<std::string>> {
		public:
			using IsOrderedSet = std::true_type;

		public:
			explicit OrderedSetType(const CacheConfiguration&)
			{}
		};

		// create a cache descriptor around BaseSetType with view and delta types that simply capture parameters
		// provide two constructors for each sub view so it can be used with and without options
		template<typename TBaseSet>
		struct TestCacheDescriptor {
			using ValueType = void;

			struct CacheViewType {
			public:
				using ReadOnlyView = void;

			public:
				explicit CacheViewType(const TBaseSet& set) : Set(set)
				{}

				CacheViewType(const TBaseSet& set, int tag)
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

				CacheDeltaType(const DeltaPointerType& pDeltaParam, int tag)
						: pDelta(pDeltaParam)
						, Tag(tag)
				{}

			public:
				// required by TestCacheOrderedExplicit
				auto pruningBoundary() const {
					return deltaset::PruningBoundary<std::string>();
				}

			public:
				DeltaPointerType pDelta;
				int Tag;
			};
		};

		// define the test caches using BasicCache
		class TestCache : public BasicCache<TestCacheDescriptor<BaseSetTypeUnorderedExplicit>, BaseSetTypeUnorderedExplicit> {
		public:
			TestCache() : BasicCache<TestCacheDescriptor<BaseSetTypeUnorderedExplicit>, BaseSetTypeUnorderedExplicit>(CacheConfiguration())
			{}
		};

		using TestCacheWithOptions = BasicCache<TestCacheDescriptor<BaseSetTypeUnorderedExplicit>, BaseSetTypeUnorderedExplicit, int>;

		class TestCacheOrderedExplicit : public BasicCache<TestCacheDescriptor<OrderedSetType>, OrderedSetType> {
		public:
			TestCacheOrderedExplicit() : BasicCache<TestCacheDescriptor<OrderedSetType>, OrderedSetType>(CacheConfiguration())
			{}
		};
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
		TestCacheWithOptions cache(CacheConfiguration(), 17);

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
		TestCacheWithOptions cache(CacheConfiguration(), 17);

		// Act:
		auto delta = cache.createDelta();

		// Assert:
		EXPECT_TRUE(!!delta.pDelta);

		EXPECT_EQ(17, delta.Tag);
	}

	TEST(TEST_CLASS, CanCreateDetachedDeltas_WithOptions) {
		// Arrange:
		TestCacheWithOptions cache(CacheConfiguration(), 17);

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
	TEST(TEST_CLASS, TEST_NAME##_OrderedExplicit) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TestCacheOrderedExplicit>(); } \
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
