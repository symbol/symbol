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

#include "src/cache/NamespaceCache.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/CacheMixinsTests.h"
#include "tests/test/cache/CachePruneTests.h"
#include "tests/test/cache/DeltaElementsMixinTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS NamespaceCacheTests

	// region mixin traits based tests

	namespace {
		struct NamespaceCacheMixinTraits {
			class CacheType : public NamespaceCache {
			public:
				CacheType() : NamespaceCache(CacheConfiguration(), NamespaceCacheTypes::Options{ BlockDuration(11) })
				{}
			};

			using IdType = NamespaceId;
			using ValueType = state::RootNamespaceHistory;

			static uint8_t GetRawId(const IdType& id) {
				return static_cast<uint8_t>(id.unwrap());
			}

			static IdType GetId(const ValueType& history) {
				return history.id();
			}

			static IdType MakeId(uint8_t id) {
				return IdType(id);
			}

			static state::RootNamespace CreateWithId(uint8_t id) {
				// RootNamespaceHistory does not move correctly with Address()
				return state::RootNamespace(MakeId(id), Address{ { 1 } }, test::CreateLifetime(234, 321));
			}

			static state::RootNamespace CreateWithIdAndExpiration(uint8_t id, Height height) {
				return state::RootNamespace(MakeId(id), Address{ { 1 } }, test::CreateLifetime(0, height.unwrap()));
			}
		};
	}

	DEFINE_CACHE_CONTAINS_TESTS(NamespaceCacheMixinTraits, ViewAccessor, _View)
	DEFINE_CACHE_CONTAINS_TESTS(NamespaceCacheMixinTraits, DeltaAccessor, _Delta)

	DEFINE_CACHE_ITERATION_TESTS(NamespaceCacheMixinTraits, ViewAccessor, _View)

	DEFINE_CACHE_TOUCH_TESTS(NamespaceCacheMixinTraits, _Delta)

	DEFINE_DELTA_ELEMENTS_MIXIN_TESTS(NamespaceCacheMixinTraits, _Delta)

	DEFINE_CACHE_BASIC_TESTS(NamespaceCacheMixinTraits,)

	// (accessors and predicates have custom tests because they're depedent on multiple caches)

	// endregion

	// *** custom tests ***

	namespace {
		void AddRoots(
				LockedCacheDelta<NamespaceCacheDelta>& delta,
				const Address& rootOwner,
				const std::vector<NamespaceId::ValueType>& ids) {
			for (auto id : ids)
				delta->insert(state::RootNamespace(NamespaceId(id), rootOwner, test::CreateLifetime(234, 321)));
		}

		void AddChildren(
				LockedCacheDelta<NamespaceCacheDelta>& delta,
				const state::RootNamespace& root,
				const std::vector<NamespaceId::ValueType>& ids) {
			for (auto id : ids)
				delta->insert(state::Namespace(test::CreatePath({ root.id().unwrap(), id })));
		}

		void PopulateCache(LockedCacheDelta<NamespaceCacheDelta>& delta, const Address& rootOwner) {
			AddRoots(delta, rootOwner, { 1, 3, 5, 7, 9 });
			AddChildren(delta, delta->find(NamespaceId(1)).get().root(), { 2, 4, 6, 8 });
			AddChildren(delta, delta->find(NamespaceId(3)).get().root(), { 10 });

			// root with id 1 is renewed
			delta->insert(delta->find(NamespaceId(1)).get().root().renew(test::CreateLifetime(345, 456)));
		}
	}

	// region properties

	TEST(TEST_CLASS, CacheWrappersExposeGracePeriodDuration) {
		// Arrange:
		NamespaceCache cache(CacheConfiguration(), NamespaceCacheTypes::Options{ BlockDuration(44) });

		// Act + Assert:
		EXPECT_EQ(BlockDuration(44), cache.createView()->gracePeriodDuration());
		EXPECT_EQ(BlockDuration(44), cache.createDelta()->gracePeriodDuration());
		EXPECT_EQ(BlockDuration(44), cache.createDetachedDelta().tryLock()->gracePeriodDuration());
	}

	// endregion

	// region deep size

	TEST(TEST_CLASS, DeepSizeRespectsRootHistory) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		{
			// - insert root with 2 children, then renew root
			auto delta = cache.createDelta();
			auto owner = test::CreateRandomOwner();
			state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));
			delta->insert(root);
			delta->insert(state::Namespace(test::CreatePath({ 123, 127 })));
			delta->insert(state::Namespace(test::CreatePath({ 123, 128 })));
			state::RootNamespace renewedRoot(NamespaceId(123), owner, test::CreateLifetime(345, 456));
			delta->insert(renewedRoot);

			// Assert: root + 2 children, one renewal
			test::AssertCacheSizes(*delta, 1, 3, 6);

			cache.commit();
		}

		// Assert: root + 2 children, one renewal
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 1, 3, 6);
	}

	TEST(TEST_CLASS, DeepSizeDoubleCountsNewChildrenAddedToSubsequentRoots) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		{
			// - insert root with 1 child
			auto delta = cache.createDelta();
			auto owner = test::CreateRandomOwner();
			state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));
			delta->insert(root);
			delta->insert(state::Namespace(test::CreatePath({ 123, 127 })));

			// - renew root and add another child
			state::RootNamespace renewedRoot(NamespaceId(123), owner, test::CreateLifetime(345, 456));
			delta->insert(renewedRoot);
			delta->insert(state::Namespace(test::CreatePath({ 123, 128 })));

			// - renew root again and add another child
			state::RootNamespace renewedRoot2(NamespaceId(123), owner, test::CreateLifetime(567, 789));
			delta->insert(renewedRoot2);
			delta->insert(state::Namespace(test::CreatePath({ 123, 129 })));

			// Assert: 3 roots x 3 children
			test::AssertCacheSizes(*delta, 1, 4, 12);

			cache.commit();
		}

		// Assert: 3 roots x 3 children
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 1, 4, 12);
	}

	TEST(TEST_CLASS, DeepSizeReturnsExpectedSizeForRootWithoutChildren) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		{
			// - insert root
			auto delta = cache.createDelta();
			auto owner = test::CreateRandomOwner();
			state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));
			delta->insert(root);

			// Assert: one root, no children
			test::AssertCacheSizes(*delta, 1, 1, 1);

			cache.commit();
		}

		// Assert: one root, no children
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 1, 1, 1);
	}

	TEST(TEST_CLASS, DeepSizeReturnsExpectedSizeForRootWithChildren) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		{
			// - insert root
			auto delta = cache.createDelta();
			auto owner = test::CreateRandomOwner();
			state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));
			delta->insert(root);
			delta->insert(state::Namespace(test::CreatePath({ 123, 127 })));
			delta->insert(state::Namespace(test::CreatePath({ 123, 128 })));

			// Assert: one root + 2 children
			test::AssertCacheSizes(*delta, 1, 3, 3);

			cache.commit();
		}

		// Assert: one root + 2 children
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 1, 3, 3);
	}

	// endregion

	// region DELTA_VIEW_BASED_TEST

	namespace {
		struct ViewTraits {
			template<typename TAction>
			static void RunTest(TAction action) {
				// Arrange:
				NamespaceCacheMixinTraits::CacheType cache;
				auto owner = test::CreateRandomOwner();
				{
					auto delta = cache.createDelta();
					PopulateCache(delta, owner);
					cache.commit();
				}

				// Act:
				auto view = cache.createView();
				action(view);
			}
		};

		struct DeltaTraits {
			template<typename TAction>
			static void RunTest(TAction action) {
				// Arrange:
				NamespaceCacheMixinTraits::CacheType cache;
				auto owner = test::CreateRandomOwner();
				auto delta = cache.createDelta();
				PopulateCache(delta, owner);

				// Act:
				action(delta);
			}
		};
	}

#define DELTA_VIEW_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_View) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ViewTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Delta) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<DeltaTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region contains

	namespace {
		void PrepareCacheForMultiLevelRootChildrenSameOwner(NamespaceCache& cache) {
			// Arrange: add two roots with one child each
			auto owner = test::CreateRandomOwner();
			{
				auto delta = cache.createDelta();
				delta->insert(state::RootNamespace(NamespaceId(123), owner, test::CreateLifetime(234, 321)));
				delta->insert(state::Namespace(test::CreatePath({ 123, 111 })));

				// Act: renew root once
				delta->insert(state::RootNamespace(NamespaceId(123), owner, test::CreateLifetime(345, 456)));
				delta->insert(state::Namespace(test::CreatePath({ 123, 222 })));

				cache.commit();
			}

			// Sanity:
			auto view = cache.createView();
			EXPECT_TRUE(view->contains(NamespaceId(123)));
			test::AssertCacheSizes(*view, 1, 3, 6);
		}

		void PrepareCacheForMultiLevelRootChildrenDifferentOwner(NamespaceCache& cache) {
			// Arrange: add two roots with one different and one shared child each
			{
				auto delta = cache.createDelta();
				delta->insert(state::RootNamespace(NamespaceId(123), test::CreateRandomOwner(), test::CreateLifetime(234, 321)));
				delta->insert(state::Namespace(test::CreatePath({ 123, 111 })));
				delta->insert(state::Namespace(test::CreatePath({ 123, 222 })));

				// Act: renew root once
				delta->insert(state::RootNamespace(NamespaceId(123), test::CreateRandomOwner(), test::CreateLifetime(345, 456)));
				delta->insert(state::Namespace(test::CreatePath({ 123, 222 })));
				delta->insert(state::Namespace(test::CreatePath({ 123, 333 })));

				cache.commit();
			}

			// Sanity:
			auto view = cache.createView();
			EXPECT_TRUE(view->contains(NamespaceId(123)));
			test::AssertCacheSizes(*view, 1, 3, 6);
		}
	}

	TEST(TEST_CLASS, ContainsReturnsTrueForChildrenOfPreviousRootWithSameOwner) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		PrepareCacheForMultiLevelRootChildrenSameOwner(cache);

		// Assert: all children are contained
		auto view = cache.createView();
		EXPECT_TRUE(view->contains(NamespaceId(111)));
		EXPECT_TRUE(view->contains(NamespaceId(222)));
	}

	TEST(TEST_CLASS, ContainsReturnsFalseForChildrenOfPreviousRootWithDifferentOwner) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		PrepareCacheForMultiLevelRootChildrenDifferentOwner(cache);

		// Assert: only children from the most recent root are contained
		auto view = cache.createView();
		EXPECT_FALSE(view->contains(NamespaceId(111)));
		EXPECT_TRUE(view->contains(NamespaceId(222)));
		EXPECT_TRUE(view->contains(NamespaceId(333)));
	}

	TEST(TEST_CLASS, ContainsReturnsTrueForChildrenOfPoppedRootWithSameOwner) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		PrepareCacheForMultiLevelRootChildrenSameOwner(cache);

		// Act:
		{
			auto delta = cache.createDelta();
			delta->remove(NamespaceId(123));
			cache.commit();
		}

		// Assert: all children are contained
		auto view = cache.createView();
		EXPECT_TRUE(view->contains(NamespaceId(111)));
		EXPECT_TRUE(view->contains(NamespaceId(222)));
	}

	TEST(TEST_CLASS, ContainsReturnsFalseForChildrenOfPoppedRootWithDifferentOwner) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		PrepareCacheForMultiLevelRootChildrenDifferentOwner(cache);

		// Act:
		{
			auto delta = cache.createDelta();
			delta->remove(NamespaceId(123));
			cache.commit();
		}

		// Assert: only children from the older root are contained
		auto view = cache.createView();
		EXPECT_TRUE(view->contains(NamespaceId(111)));
		EXPECT_TRUE(view->contains(NamespaceId(222)));
		EXPECT_FALSE(view->contains(NamespaceId(333)));
	}

	// endregion

	// region get / tryGet

	DELTA_VIEW_BASED_TEST(GetReturnsKnownRootNamespace) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			auto entry = view->find(NamespaceId(3)).get();

			// Assert:
			EXPECT_EQ(test::CreatePath({ 3 }), entry.ns().path());
			EXPECT_EQ(NamespaceId(3), entry.root().id());
		});
	}

	DELTA_VIEW_BASED_TEST(GetReturnsKnownChildNamespace) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			auto entry = view->find(NamespaceId(2)).get();

			// Assert:
			EXPECT_EQ(test::CreatePath({ 1, 2 }), entry.ns().path());
			EXPECT_EQ(NamespaceId(1), entry.root().id());
		});
	}

	DELTA_VIEW_BASED_TEST(GetThrowsWhenNamespaceIsUnknown) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			auto namespaceIter = view->find(NamespaceId(123));

			// Assert:
			EXPECT_THROW(namespaceIter.get(), catapult_invalid_argument);
		});
	}

	DELTA_VIEW_BASED_TEST(TryGetReturnsKnownRootNamespace) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			auto namespaceIter = view->find(NamespaceId(3));
			const auto* pEntry = namespaceIter.tryGet();

			// Assert:
			ASSERT_TRUE(!!pEntry);

			EXPECT_EQ(test::CreatePath({ 3 }), pEntry->ns().path());
			EXPECT_EQ(NamespaceId(3), pEntry->root().id());
		});
	}

	DELTA_VIEW_BASED_TEST(TryGetReturnsKnownChildNamespace) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			auto namespaceIter = view->find(NamespaceId(2));
			const auto* pEntry = namespaceIter.tryGet();

			// Assert:
			ASSERT_TRUE(!!pEntry);

			EXPECT_EQ(test::CreatePath({ 1, 2 }), pEntry->ns().path());
			EXPECT_EQ(NamespaceId(1), pEntry->root().id());
		});
	}

	DELTA_VIEW_BASED_TEST(TryGetReturnsNullptrWhenNamespaceIsUnknown) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			auto namespaceIter = view->find(NamespaceId(123));
			const auto* pEntry = namespaceIter.tryGet();

			// Assert:
			EXPECT_FALSE(!!pEntry);
		});
	}

	DELTA_VIEW_BASED_TEST(TryGetUnadaptedReturnsKnownRootNamespace) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			auto namespaceIter = view->find(NamespaceId(3));
			const auto* pHistory = namespaceIter.tryGetUnadapted();

			// Assert:
			ASSERT_TRUE(!!pHistory);

			EXPECT_EQ(1u, pHistory->historyDepth());
			EXPECT_EQ(NamespaceId(3), pHistory->id());
		});
	}

	DELTA_VIEW_BASED_TEST(TryGetUnadaptedReturnsKnownChildNamespace) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			auto namespaceIter = view->find(NamespaceId(2));
			const auto* pHistory = namespaceIter.tryGetUnadapted();

			// Assert:
			ASSERT_TRUE(!!pHistory);

			EXPECT_EQ(2u, pHistory->historyDepth());
			EXPECT_EQ(NamespaceId(1), pHistory->id()); // root id
		});
	}

	DELTA_VIEW_BASED_TEST(TryGetUnadaptedReturnsNullptrWhenNamespaceIsUnknown) {
		// Act:
		TTraits::RunTest([](const auto& view) {
			auto namespaceIter = view->find(NamespaceId(123));
			const auto* pHistory = namespaceIter.tryGetUnadapted();

			// Assert:
			EXPECT_FALSE(!!pHistory);
		});
	}

	// endregion

	// region insert

	namespace {
		void AssertLifetime(const state::NamespaceLifetime& lifetime, uint64_t expectedStart, uint64_t expectedEnd) {
			// Assert:
			EXPECT_EQ(Height(expectedStart), lifetime.Start);
			EXPECT_EQ(Height(expectedEnd), lifetime.End);
		}
	}

	TEST(TEST_CLASS, CanInsertRoot) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));

		// Act:
		delta->insert(root);

		// Assert:
		test::AssertCacheSizes(*delta, 1, 1, 1);
		EXPECT_TRUE(delta->contains(NamespaceId(123)));

		AssertLifetime(delta->find(NamespaceId(123)).get().root().lifetime(), 234, 321);
	}

	TEST(TEST_CLASS, CanRenewRoot) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto owner = test::CreateRandomOwner();
		state::RootNamespace root1(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		state::RootNamespace root2(NamespaceId(123), owner, test::CreateLifetime(345, 456));
		state::RootNamespace root3(NamespaceId(123), owner, test::CreateLifetime(456, 567));
		{
			auto delta = cache.createDelta();
			delta->insert(root1);

			// Act: renew root two times
			delta->insert(root2);
			delta->insert(root3);

			cache.commit();
		}

		// Assert:
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 1, 1, 3);
		ASSERT_TRUE(view->contains(NamespaceId(123)));
		EXPECT_EQ(root3, view->find(NamespaceId(123)).get().root());

		AssertLifetime(view->find(NamespaceId(123)).get().root().lifetime(), 456, 567);
	}

	TEST(TEST_CLASS, RenewingRootUpdatesChildNamespacesWithNewRoot) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto owner = test::CreateRandomOwner();
		state::RootNamespace originalRoot(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		state::RootNamespace newRoot(NamespaceId(123), owner, test::CreateLifetime(345, 456));
		auto delta = cache.createDelta();
		delta->insert(originalRoot);
		delta->insert(state::Namespace(test::CreatePath({ 123, 124 })));
		delta->insert(state::Namespace(test::CreatePath({ 123, 125 })));
		cache.commit();

		// Sanity:
		test::AssertCacheSizes(*delta, 1, 3, 3);
		EXPECT_EQ(originalRoot, delta->find(NamespaceId(123)).get().root());
		EXPECT_EQ(originalRoot, delta->find(NamespaceId(124)).get().root());
		EXPECT_EQ(originalRoot, delta->find(NamespaceId(125)).get().root());

		// Act: renew root
		delta->insert(newRoot);

		// Assert:
		test::AssertCacheSizes(*delta, 1, 3, 6);
		ASSERT_TRUE(delta->contains(NamespaceId(123)));
		ASSERT_TRUE(delta->contains(NamespaceId(124)));
		ASSERT_TRUE(delta->contains(NamespaceId(125)));
		EXPECT_EQ(newRoot, delta->find(NamespaceId(123)).get().root());
		EXPECT_EQ(newRoot, delta->find(NamespaceId(124)).get().root());
		EXPECT_EQ(newRoot, delta->find(NamespaceId(125)).get().root());

		AssertLifetime(delta->find(NamespaceId(123)).get().root().lifetime(), 345, 456);
	}

	TEST(TEST_CLASS, CanInsertSingleChildWhenRootIsKnown) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		delta->insert(root);

		// Act:
		delta->insert(state::Namespace(test::CreatePath({ 123, 127 })));

		// Assert:
		test::AssertCacheSizes(*delta, 1, 2, 2);
		EXPECT_TRUE(delta->contains(NamespaceId(127)));

		// Sanity: child should not affect namespace
		AssertLifetime(delta->find(NamespaceId(123)).get().root().lifetime(), 234, 321);
	}

	TEST(TEST_CLASS, CanAbandonInsertSingleChildWhenRootIsKnown) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		{
			// - add and commit a root
			auto delta = cache.createDelta();
			auto owner = test::CreateRandomOwner();
			state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));
			delta->insert(root);
			cache.commit();

			// Act: add but do not commit a child
			delta->insert(state::Namespace(test::CreatePath({ 123, 127 })));
		}

		// Assert: the child was not added (only the root is present)
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 1, 1, 1);
		EXPECT_FALSE(view->contains(NamespaceId(127)));
		EXPECT_TRUE(view->contains(NamespaceId(123)));
	}

	TEST(TEST_CLASS, CanInsertMultipleChildrenWhenParentsAreKnown) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		state::RootNamespace root1(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		state::RootNamespace root2(NamespaceId(124), owner, test::CreateLifetime(234, 321));
		delta->insert(root1);
		delta->insert(root2);

		// Act:
		delta->insert(state::Namespace(test::CreatePath({ 123, 127 })));
		delta->insert(state::Namespace(test::CreatePath({ 123, 127, 128 })));
		delta->insert(state::Namespace(test::CreatePath({ 124, 125 })));

		// Assert: 2 roots and 3 children
		test::AssertCacheSizes(*delta, 2, 5, 5);
		EXPECT_TRUE(delta->contains(NamespaceId(127)));
		EXPECT_TRUE(delta->contains(NamespaceId(128)));
		EXPECT_TRUE(delta->contains(NamespaceId(125)));
	}

	TEST(TEST_CLASS, CannotInsertChildWhenParentIsUnknown) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));
		delta->insert(root);

		// Act + Assert:
		EXPECT_THROW(delta->insert(state::Namespace(test::CreatePath({ 123, 126, 127 }))), catapult_invalid_argument);
		EXPECT_THROW(delta->insert(state::Namespace(test::CreatePath({ 125, 127 }))), catapult_invalid_argument);
		EXPECT_THROW(delta->insert(state::Namespace(test::CreatePath({ 122 }))), catapult_invalid_argument);
	}

	// endregion

	// region setAlias

	namespace {
		state::NamespaceAlias GetNamespaceAliasFromCache(const NamespaceCacheView& view, NamespaceId id) {
			return view.find(id).get().root().alias(id);
		}

		void AssertCanSetNamespace(NamespaceId namespaceId) {
			// Arrange: insert a root with child
			NamespaceCacheMixinTraits::CacheType cache;
			auto owner = test::CreateRandomOwner();
			state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));

			{
				auto delta = cache.createDelta();
				delta->insert(root);
				delta->insert(state::Namespace(test::CreatePath({ 123, 127 })));

				// Sanity:
				test::AssertCacheSizes(*delta, 1, 2, 2);
				EXPECT_TRUE(delta->contains(NamespaceId(127)));

				// Act:
				delta->setAlias(namespaceId, state::NamespaceAlias(MosaicId(444)));
				cache.commit();
			}

			auto view = cache.createView();
			auto alias = GetNamespaceAliasFromCache(*view, namespaceId);

			// Assert:
			EXPECT_EQ(state::AliasType::Mosaic, alias.type());
			EXPECT_EQ(MosaicId(444), alias.mosaicId());
		}
	}

	TEST(TEST_CLASS, CanSetRootNamespaceAlias) {
		AssertCanSetNamespace(NamespaceId(123));
	}

	TEST(TEST_CLASS, CanSetChildNamespaceAlias) {
		AssertCanSetNamespace(NamespaceId(127));
	}

	TEST(TEST_CLASS, CannotSetUnknownNamespaceAlias) {
		// Arrange: insert a root with child
		NamespaceCacheMixinTraits::CacheType cache;
		auto owner = test::CreateRandomOwner();
		state::RootNamespace root(NamespaceId(123), owner, test::CreateLifetime(234, 321));

		auto delta = cache.createDelta();
		delta->insert(root);
		delta->insert(state::Namespace(test::CreatePath({ 123, 127 })));

		// Sanity:
		test::AssertCacheSizes(*delta, 1, 2, 2);
		EXPECT_TRUE(delta->contains(NamespaceId(127)));

		// Act + Assert:
		EXPECT_THROW(delta->setAlias(NamespaceId(129), state::NamespaceAlias(MosaicId(444))), catapult_invalid_argument);
	}

	// endregion

	// region remove

	TEST(TEST_CLASS, CannotRemoveUnknownNamespace) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		PopulateCache(delta, owner);

		// Act + Assert:
		EXPECT_THROW(delta->remove(NamespaceId(12)), catapult_invalid_argument);
		EXPECT_THROW(delta->remove(NamespaceId(123)), catapult_invalid_argument);
		EXPECT_THROW(delta->remove(NamespaceId(3579)), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanRemoveChildNamespace) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		PopulateCache(delta, owner);

		// Sanity:
		test::AssertCacheSizes(*delta, 5, 10, 15);
		EXPECT_TRUE(delta->contains(NamespaceId(2)));
		EXPECT_FALSE(delta->find(NamespaceId(2)).get().ns().isRoot());
		EXPECT_EQ(4u, delta->find(NamespaceId(2)).get().root().size());

		// Act:
		delta->remove(NamespaceId(2));

		// Assert:
		test::AssertCacheSizes(*delta, 5, 9, 13); // note that child is removed from all (two) roots in history
		EXPECT_FALSE(delta->contains(NamespaceId(2)));
		EXPECT_EQ(3u, delta->find(NamespaceId(1)).get().root().size());
	}

	TEST(TEST_CLASS, CanAbandonRemoveChildNamespace) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		{
			// - populate the cache
			auto delta = cache.createDelta();
			auto owner = test::CreateRandomOwner();
			PopulateCache(delta, owner);
			cache.commit();

			// Act: remove but do not commit a child
			delta->remove(NamespaceId(2));
		}

		// Assert: the child was not removed
		auto view = cache.createView();
		test::AssertCacheSizes(*view, 5, 10, 15);
		EXPECT_TRUE(view->contains(NamespaceId(2)));
		EXPECT_EQ(4u, view->find(NamespaceId(1)).get().root().size());
	}

	TEST(TEST_CLASS, CanRemoveRootNamespaceWithoutChildren) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		PopulateCache(delta, owner);

		// Sanity:
		test::AssertCacheSizes(*delta, 5, 10, 15);
		ASSERT_TRUE(delta->contains(NamespaceId(5)));
		EXPECT_TRUE(delta->find(NamespaceId(5)).get().ns().isRoot());
		EXPECT_TRUE(delta->find(NamespaceId(5)).get().root().empty());

		// Act: root with id 5 has no children and therefore can be removed
		delta->remove(NamespaceId(5));

		// Assert:
		test::AssertCacheSizes(*delta, 4, 9, 14);
		EXPECT_FALSE(delta->contains(NamespaceId(5)));
	}

	TEST(TEST_CLASS, CanRemoveRootNamespaceWithChildrenWhenHistoryDepthIsNotOne) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		PopulateCache(delta, owner);

		// Sanity:
		test::AssertCacheSizes(*delta, 5, 10, 15);
		ASSERT_TRUE(delta->contains(NamespaceId(1)));
		EXPECT_EQ(4u, delta->find(NamespaceId(1)).get().root().size());

		// Act: root with id 1 has four children, the namespace that we remove has inherited the children via renewal
		delta->remove(NamespaceId(1));

		// Assert: the old root with id 1 is still present and has all four children
		test::AssertCacheSizes(*delta, 5, 10, 10);
		ASSERT_TRUE(delta->contains(NamespaceId(1)));
		EXPECT_EQ(4u, delta->find(NamespaceId(1)).get().root().size());
	}

	TEST(TEST_CLASS, RemovingRootNamespaceWhenHistoryDepthIsNotOneUpdatesChildNamespacesWithOldRoot) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		PopulateCache(delta, owner);

		// Sanity:
		test::AssertCacheSizes(*delta, 5, 10, 15);
		ASSERT_TRUE(delta->contains(NamespaceId(1)));
		EXPECT_EQ(4u, delta->find(NamespaceId(1)).get().root().size());

		// - all children should have the most recent root as member
		for (const auto& pair : delta->find(NamespaceId(1)).get().root().children())
			EXPECT_EQ(Height(345), delta->find(pair.first).get().root().lifetime().Start);

		// Act:
		delta->remove(NamespaceId(1));

		// Assert: the old root with id 1 is still present and has all four children
		test::AssertCacheSizes(*delta, 5, 10, 10);
		ASSERT_TRUE(delta->contains(NamespaceId(1)));
		EXPECT_EQ(4u, delta->find(NamespaceId(1)).get().root().size());

		// - all children should have the old root as member
		for (const auto& pair : delta->find(NamespaceId(1)).get().root().children())
			EXPECT_EQ(Height(234), delta->find(pair.first).get().root().lifetime().Start);
	}

	TEST(TEST_CLASS, CanRemoveRootNamespaceWithoutChildrenWhenHistoryDepthIsNotOne) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		PopulateCache(delta, owner);

		// - renew root with id 5
		delta->insert(delta->find(NamespaceId(5)).get().root().renew(test::CreateLifetime(567, 678)));

		// Sanity:
		test::AssertCacheSizes(*delta, 5, 10, 16);
		ASSERT_TRUE(delta->contains(NamespaceId(5)));
		EXPECT_TRUE(delta->find(NamespaceId(5)).get().root().empty());

		// Act: namespace with id 5 has no children
		delta->remove(NamespaceId(5));

		// Assert:
		test::AssertCacheSizes(*delta, 5, 10, 15);
		ASSERT_TRUE(delta->contains(NamespaceId(5)));
		EXPECT_TRUE(delta->find(NamespaceId(5)).get().root().empty());
	}

	TEST(TEST_CLASS, CannotRemoveRootNamespaceWithChildrenWhenHistoryDepthIsOne) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto delta = cache.createDelta();
		auto owner = test::CreateRandomOwner();
		PopulateCache(delta, owner);

		// Sanity:
		test::AssertCacheSizes(*delta, 5, 10, 15);
		ASSERT_TRUE(delta->contains(NamespaceId(3)));
		EXPECT_FALSE(delta->find(NamespaceId(3)).get().root().empty());

		// Act + Assert: namespace with id 3 has 1 child
		EXPECT_THROW(delta->remove(NamespaceId(3)), catapult_runtime_error);
	}

	TEST(TEST_CLASS, RemoveRestoresAliasesFromHistory) {
		// Arrange: add two roots with one different and one shared child each
		NamespaceCacheMixinTraits::CacheType cache;
		{
			auto delta = cache.createDelta();
			delta->insert(state::RootNamespace(NamespaceId(123), test::CreateRandomOwner(), test::CreateLifetime(234, 321)));
			delta->insert(state::Namespace(test::CreatePath({ 123, 111 })));
			delta->insert(state::Namespace(test::CreatePath({ 123, 222 })));
			delta->setAlias(NamespaceId(123), state::NamespaceAlias(MosaicId(1)));
			delta->setAlias(NamespaceId(111), state::NamespaceAlias(MosaicId(2)));
			delta->setAlias(NamespaceId(222), state::NamespaceAlias(MosaicId(3)));

			// Act: renew root once
			delta->insert(state::RootNamespace(NamespaceId(123), test::CreateRandomOwner(), test::CreateLifetime(345, 456)));
			delta->insert(state::Namespace(test::CreatePath({ 123, 222 })));
			delta->insert(state::Namespace(test::CreatePath({ 123, 333 })));
			delta->setAlias(NamespaceId(123), state::NamespaceAlias(MosaicId(4)));
			delta->setAlias(NamespaceId(222), state::NamespaceAlias(MosaicId(5)));
			delta->setAlias(NamespaceId(333), state::NamespaceAlias(MosaicId(6)));

			cache.commit();
		}

		// Sanity:
		{
			auto view = cache.createView();
			EXPECT_EQ(MosaicId(4), GetNamespaceAliasFromCache(*view, NamespaceId(123)).mosaicId());
			EXPECT_EQ(MosaicId(5), GetNamespaceAliasFromCache(*view, NamespaceId(222)).mosaicId());
			EXPECT_EQ(MosaicId(6), GetNamespaceAliasFromCache(*view, NamespaceId(333)).mosaicId());
		}

		// Act:
		{
			auto delta = cache.createDelta();
			delta->remove(NamespaceId(123));
			cache.commit();
		}

		// Assert: only children from the older root are contained
		auto view = cache.createView();
		EXPECT_EQ(MosaicId(1), GetNamespaceAliasFromCache(*view, NamespaceId(123)).mosaicId());
		EXPECT_EQ(MosaicId(2), GetNamespaceAliasFromCache(*view, NamespaceId(111)).mosaicId());
		EXPECT_EQ(MosaicId(3), GetNamespaceAliasFromCache(*view, NamespaceId(222)).mosaicId());
	}

	namespace {
		bool Contains(const BasicNamespaceCacheDelta::CollectedIds& ids, NamespaceId id) {
			return ids.cend() != ids.find(id);
		}
	}

	TEST(TEST_CLASS, RemoveRootNamespaceRemovesNamespaceIdFromHeightBasedMapWhenHistoryDepthIsOne) {
		// Arrange: namespace with id 5 expires at height 321
		NamespaceCacheMixinTraits::CacheType cache;
		auto owner = test::CreateRandomOwner();
		{
			auto delta = cache.createDelta();
			PopulateCache(delta, owner);

			// Sanity:
			test::AssertCacheSizes(*delta, 5, 10, 15);
			ASSERT_TRUE(delta->contains(NamespaceId(5)));
			EXPECT_TRUE(delta->find(NamespaceId(5)).get().root().empty());

			cache.commit();
		}

		{
			// Act: remove namespace with id 5
			auto delta = cache.createDelta();
			delta->remove(NamespaceId(5));
			cache.commit();
		}

		// Act: reinsert namespace with id 5 and expiry height 123
		// - note that the new expiry height has to be lower so that pruning will see an expired root at height 321
		auto delta = cache.createDelta();
		auto newOwner = test::CreateRandomOwner();
		delta->insert(state::RootNamespace(NamespaceId(5), newOwner, test::CreateLifetime(100, 123)));

		// Assert:
		auto removedIds1 = delta->prune(Height(321));
		EXPECT_FALSE(Contains(removedIds1, NamespaceId(5)));

		auto removedIds2 = delta->prune(Height(123));
		EXPECT_CONTAINS(removedIds2, NamespaceId(5));
	}

	TEST(TEST_CLASS, RemoveRootNamespaceRemovesNamespaceIdFromHeightBasedMapWhenHistoryDepthIsGreaterThanOne) {
		// Arrange: namespace with id 5 expires at height 321
		NamespaceCacheMixinTraits::CacheType cache;
		auto owner = test::CreateRandomOwner();
		{
			auto delta = cache.createDelta();
			PopulateCache(delta, owner);

			// Sanity:
			test::AssertCacheSizes(*delta, 5, 10, 15);
			ASSERT_TRUE(delta->contains(NamespaceId(5)));
			EXPECT_TRUE(delta->find(NamespaceId(5)).get().root().empty());

			cache.commit();
		}

		// Act: renew namespace with id 5 and expiry height 432
		{
			auto delta = cache.createDelta();
			delta->insert(state::RootNamespace(NamespaceId(5), owner, test::CreateLifetime(325, 432)));
			cache.commit();
		}

		{
			// - remove namespace with id 5
			auto delta = cache.createDelta();
			delta->remove(NamespaceId(5));
			cache.commit();
		}

		{
			// - renew namespace with id 5 and expiry height 345
			auto delta = cache.createDelta();
			delta->insert(state::RootNamespace(NamespaceId(5), owner, test::CreateLifetime(325, 345)));
			cache.commit();
		}

		{
			// - prune at height 321
			auto delta = cache.createDelta();
			delta->prune(Height(321));
			cache.commit();
		}

		// Sanity:
		auto delta = cache.createDelta();
		EXPECT_EQ(Height(345), delta->find(NamespaceId(5)).get().root().lifetime().End);

		// Assert: if the height based map would contain an entry at height 432, then the last root
		//         would be pruned and the root id would be collected
		auto removedIds1 = delta->prune(Height(432));
		EXPECT_FALSE(Contains(removedIds1, NamespaceId(5)));

		// - the height based map does contain an entry at height 345
		auto removedIds2 = delta->prune(Height(345));
		EXPECT_CONTAINS(removedIds2, NamespaceId(5));
	}

	// endregion

	// region prune

	DEFINE_CACHE_PRUNE_TESTS(NamespaceCacheMixinTraits,)

	namespace {
		void SetupCacheForPruneTests(NamespaceCache& cache, const Address& rootOwner) {
			// 5 roots with id i and lifetime (10 * i, 10 * (i + 1)) for i = 0 ... 4 (each root has 2 children)
			constexpr size_t Root_Count = 5;

			auto delta = cache.createDelta();
			for (auto i = 0u; i < Root_Count; ++i) {
				delta->insert(state::RootNamespace(NamespaceId(i), rootOwner, test::CreateLifetime(10 * i, 10 * (i + 1))));
				delta->insert(state::Namespace(test::CreatePath({ i, 10 + i })));
				delta->insert(state::Namespace(test::CreatePath({ i, 20 + i })));
			}

			cache.commit();
		}

		void RenewSameOwner(NamespaceCache& cache) {
			// renew namespace with id 0 and add a child to it
			auto delta = cache.createDelta();
			delta->insert(delta->find(NamespaceId(0)).get().root().renew(test::CreateLifetime(100, 110)));
			delta->insert(state::Namespace(test::CreatePath({ 0, 30 })));
			cache.commit();
		}

		void RenewDifferentOwner(NamespaceCache& cache, const Address& diffOwner) {
			// renew namespace with id 4 having a different owner and add a child to it
			// note that since it is a different owner, the previous two children of the namespace are 'hidden'
			// and are not counted in activeSize()
			auto delta = cache.createDelta();
			delta->insert(state::RootNamespace(NamespaceId(4), diffOwner, test::CreateLifetime(120, 130)));
			delta->insert(state::Namespace(test::CreatePath({ 4, 34 })));
			cache.commit();
		}

		using NamespaceIds = NamespaceCacheDelta::CollectedIds;

		void AssertPrunedIds(std::initializer_list<NamespaceId::ValueType> expected, const NamespaceIds& prunedIds) {
			// Assert:
			EXPECT_EQ(expected.size(), prunedIds.size());
			for (auto id : expected)
				EXPECT_CONTAINS(prunedIds, NamespaceId(id));
		}
	}

	TEST(TEST_CLASS, PruneRemovesExpiredNamespacesWhenHistoryDepthIsOne) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto owner = test::CreateRandomOwner();
		SetupCacheForPruneTests(cache, owner);
		NamespaceIds prunedIds;
		{
			auto delta = cache.createDelta();

			// Sanity:
			test::AssertCacheSizes(*delta, 5, 15, 15);

			// Act: prune root with id 2 and the associated children
			prunedIds = delta->prune(Height(30));
			cache.commit();

			// Assert:
			test::AssertCacheSizes(*delta, 4, 12, 12);
		}

		test::AssertCacheContents(cache, { 0, 10, 20, 1, 11, 21, 3, 13, 23, 4, 14, 24 });
		AssertPrunedIds({ 2, 12, 22 }, prunedIds);
	}

	TEST(TEST_CLASS, PruneRemovesExpiredNamespacesWhenHistoryDepthIsNotOne) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto owner = test::CreateRandomOwner();
		SetupCacheForPruneTests(cache, owner);
		RenewSameOwner(cache);
		NamespaceIds prunedIds;
		{
			auto delta = cache.createDelta();

			// Sanity:
			test::AssertCacheSizes(*delta, 5, 16, 20);

			// Act: prune root with id 0 (note that only the old root 0 is pruned, all children are protected by the newer version)
			prunedIds = delta->prune(Height(10));
			cache.commit();

			// Assert:
			test::AssertCacheSizes(*delta, 5, 16, 16);
		}

		test::AssertCacheContents(cache, { 0, 10, 20, 30, 1, 11, 21, 2, 12, 22, 3, 13, 23, 4, 14, 24 });
		AssertPrunedIds({}, prunedIds);
	}

	TEST(TEST_CLASS, PruneRemovesChildrenOfOldExpiredRootWithDifferentOwner) {
		// Arrange:
		NamespaceCacheMixinTraits::CacheType cache;
		auto owner = test::CreateRandomOwner();
		auto diffOwner = test::CreateRandomOwner();
		SetupCacheForPruneTests(cache, owner);
		RenewDifferentOwner(cache, diffOwner);
		std::vector<NamespaceIds> prunedIdGroups;
		{
			auto delta = cache.createDelta();

			// Sanity:
			test::AssertCacheSizes(*delta, 5, 14, 17);

			// Act: prune all roots at their original expiration heights
			//      the old root with id 4 had two children (that get pruned) and the renewed root has one child (that stays)
			for (auto height = Height(10); height <= Height(50); height = height + Height(10))
				prunedIdGroups.push_back(delta->prune(height));

			cache.commit();

			// Assert:
			test::AssertCacheSizes(*delta, 1, 2, 2);
		}

		test::AssertCacheContents(cache, { 4, 34 });

		AssertPrunedIds({ 0, 10, 20 }, prunedIdGroups[0]);
		AssertPrunedIds({ 1, 11, 21 }, prunedIdGroups[1]);
		AssertPrunedIds({ 2, 12, 22 }, prunedIdGroups[2]);
		AssertPrunedIds({ 3, 13, 23 }, prunedIdGroups[3]);
		AssertPrunedIds({ 14, 24 }, prunedIdGroups[4]);
	}

	TEST(TEST_CLASS, PruneOnlyRemovesExpiredNamespacesWithGracePeriodEndHeightLessOrEqualToPruneHeight) {
		// Arrange:
		auto owner = test::CreateRandomOwner();
		NamespaceCacheMixinTraits::CacheType cache;
		{
			auto delta = cache.createDelta();
			delta->insert(state::RootNamespace(NamespaceId(5), owner, test::CreateLifetime(100, 120)));
			delta->insert(state::RootNamespace(NamespaceId(5), owner, test::CreateLifetime(120, 121)));
			delta->insert(state::RootNamespace(NamespaceId(6), owner, test::CreateLifetime(100, 120)));
			delta->insert(state::RootNamespace(NamespaceId(6), owner, test::CreateLifetime(120, 131)));
			delta->insert(state::RootNamespace(NamespaceId(7), owner, test::CreateLifetime(100, 110)));
			delta->insert(state::RootNamespace(NamespaceId(7), owner, test::CreateLifetime(110, 120)));
			cache.commit();
		}

		NamespaceIds prunedIds;
		{
			auto delta = cache.createDelta();

			// Sanity:
			test::AssertCacheSizes(*delta, 3, 3, 6);

			// Act: note that only the roots with expiry height less than or equal to 120 should be pruned
			prunedIds = delta->prune(Height(120));
			cache.commit();

			// Assert:
			test::AssertCacheSizes(*delta, 2, 2, 2);
		}

		test::AssertCacheContents(cache, { 5, 6 });
		AssertPrunedIds({ 7 }, prunedIds);
	}

	// endregion

	// region cache init

	TEST(TEST_CLASS, CanSpecifyInitialValuesViaInit) {
		// Arrange:
		auto config = CacheConfiguration();
		NamespaceCache cache(config, NamespaceCacheTypes::Options());

		// Act:
		cache.init(static_cast<size_t>(12), static_cast<size_t>(7));

		// Assert:
		auto view = cache.createView();
		EXPECT_EQ(12u, view->activeSize());
		EXPECT_EQ(7u, view->deepSize());
	}

	// endregion
}}
