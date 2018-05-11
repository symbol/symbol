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

#include "catapult/cache/SubCachePluginAdapter.h"
#include "catapult/cache/CatapultCache.h"
#include "tests/test/cache/CacheBasicTests.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS SubCachePluginAdapterTests

	namespace {
		using SimpleCache = test::SimpleCacheT<3>;
		using SimpleCachePluginAdapter = SubCachePluginAdapter<SimpleCache, test::SimpleCacheStorageTraits>;

		std::unique_ptr<SimpleCache> CreateSimpleCacheWithValue(
				size_t value,
				test::SimpleCacheViewMode mode = test::SimpleCacheViewMode::Iterable) {
			auto pCache = std::make_unique<SimpleCache>(mode);
			auto delta = pCache->createDelta();
			for (auto i = 0u; i < value; ++i)
				delta->increment();

			pCache->commit();
			return pCache;
		}

		template<typename TRawView>
		void AssertConstSubCacheView(const SubCacheView& view, size_t expectedValue) {
			// Assert: const raw view is correct
			ASSERT_TRUE(!!view.get());
			EXPECT_EQ(expectedValue, static_cast<const TRawView*>(view.get())->id());

			// - read only view is correct
			const auto* pReadOnlyView = static_cast<const test::SimpleCacheReadOnlyType*>(view.asReadOnly());
			ASSERT_TRUE(!!pReadOnlyView);
			EXPECT_EQ(expectedValue, pReadOnlyView->size());
		}

		template<typename TRawView>
		void AssertNonConstSubCacheView(SubCacheView& view, size_t expectedValue) {
			// Assert: non-const raw view is correct
			ASSERT_TRUE(!!view.get());
			EXPECT_EQ(expectedValue, static_cast<TRawView*>(view.get())->id());

			// - raw views are equal
			const auto& constView = view;
			EXPECT_EQ(view.get(), constView.get());
		}

		template<typename TRawView>
		void AssertView(const std::unique_ptr<const SubCacheView>& pView, size_t expectedValue) {
			// Assert: a valid view was returned
			ASSERT_TRUE(!!pView);
			AssertConstSubCacheView<TRawView>(*pView, expectedValue);
		}

		template<typename TRawView>
		void AssertView(const std::unique_ptr<SubCacheView>& pView, size_t expectedValue) {
			// Assert: a valid view was returned
			ASSERT_TRUE(!!pView);
			AssertConstSubCacheView<TRawView>(*pView, expectedValue);
			AssertNonConstSubCacheView<TRawView>(*pView, expectedValue);
		}
	}

	// region constructor / name

	TEST(TEST_CLASS, CanAccessRawCachePointer) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act:
		const auto* pCache = static_cast<const SimpleCache*>(adapter.get());

		// Assert:
		ASSERT_TRUE(!!pCache);
		EXPECT_EQ(5u, pCache->createView()->id());
	}

	TEST(TEST_CLASS, CanAccessCacheName) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act:
		auto name = adapter.name();

		// Assert:
		EXPECT_EQ("SimpleCache (id = 3)", name);
	}

	// endregion

	// region createView / createDelta

	TEST(TEST_CLASS, CanAccessView) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act:
		auto pView = adapter.createView();

		// Assert:
		AssertView<test::SimpleCacheView>(pView, 5);
	}

	TEST(TEST_CLASS, CanAccessDelta) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act:
		auto pDelta = adapter.createDelta();

		// Assert:
		AssertView<test::SimpleCacheDelta>(pDelta, 5);
	}

	// endregion

	// region createDetachedDelta

	TEST(TEST_CLASS, CanAccessDetachedDelta) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act:
		auto pDetachedDelta = adapter.createDetachedDelta();
		ASSERT_TRUE(!!pDetachedDelta);
	}

	TEST(TEST_CLASS, CanAccessDeltaViewViaDetachedDelta) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));
		auto pDetachedDelta = adapter.createDetachedDelta();
		ASSERT_TRUE(!!pDetachedDelta);

		// Act:
		auto pDelta = pDetachedDelta->lock();

		// Assert:
		AssertView<test::SimpleCacheDelta>(pDelta, 5);
	}

	TEST(TEST_CLASS, CannotAccessDeltaViewViaOutdatedDetachedDelta) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));
		auto pDetachedDelta = adapter.createDetachedDelta();
		ASSERT_TRUE(!!pDetachedDelta);

		{
			// - commit a change
			auto pDelta = adapter.createDelta();
			auto pDeltaRaw = static_cast<test::SimpleCacheDelta*>(pDelta->get());
			pDeltaRaw->increment();
			adapter.commit();
		}

		// Act:
		auto pDelta = pDetachedDelta->lock();

		// Assert:
		EXPECT_FALSE(!!pDelta);
	}

	// endregion

	// region commit

	TEST(TEST_CLASS, CanDiscardNonCommittedChanges) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));
		{
			auto pDelta = adapter.createDelta();
			auto pDeltaRaw = static_cast<test::SimpleCacheDelta*>(pDelta->get());
			pDeltaRaw->increment();
		}

		// Act:
		auto pView = adapter.createView();

		// Assert: the increment above was discarded
		AssertView<test::SimpleCacheView>(pView, 5);
	}

	TEST(TEST_CLASS, CanCommitChanges) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));
		{
			auto pDelta = adapter.createDelta();
			auto pDeltaRaw = static_cast<test::SimpleCacheDelta*>(pDelta->get());
			pDeltaRaw->increment();
			adapter.commit();
		}

		// Act:
		auto pView = adapter.createView();

		// Assert: the increment above was committed
		AssertView<test::SimpleCacheView>(pView, 6);
	}

	// endregion

	// region createStorage

	TEST(TEST_CLASS, CanAccessStorageWhenCacheSupportsIteration) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(0, test::SimpleCacheViewMode::Iterable));

		// Act:
		auto pCacheStorage = adapter.createStorage();

		// Assert:
		ASSERT_TRUE(!!pCacheStorage);
	}

	TEST(TEST_CLASS, CannotAccessStorageWhenCacheDoesNotSupportIteration) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(0, test::SimpleCacheViewMode::Non_Iterable));

		// Act:
		auto pCacheStorage = adapter.createStorage();

		// Assert:
		ASSERT_FALSE(!!pCacheStorage);
	}

	TEST(TEST_CLASS, CanSerializeCacheToStorage) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));
		auto pCacheStorage = adapter.createStorage();
		ASSERT_TRUE(!!pCacheStorage);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		// Act:
		pCacheStorage->saveAll(stream);

		// Assert:
		ASSERT_EQ(6 * sizeof(uint64_t), buffer.size());

		const auto* pData64 = reinterpret_cast<const uint64_t*>(buffer.data());
		EXPECT_EQ(5u, pData64[0]); // size;

		for (auto i = 1u; i <= 5; ++i)
			EXPECT_EQ(i ^ 0xFFFFFFFF'FFFFFFFF, pData64[i]) << "value at " << i;
	}

	TEST(TEST_CLASS, CanDeserializeCacheFromStorage) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(0));
		auto pCacheStorage = adapter.createStorage();
		ASSERT_TRUE(!!pCacheStorage);

		// - prepare the input
		std::vector<uint8_t> buffer(4 * sizeof(uint64_t));
		auto* pData64 = reinterpret_cast<uint64_t*>(buffer.data());
		pData64[0] = 3; // size
		for (auto i = 1u; i <= 3; ++i)
			pData64[i] = i ^ 0xFFFFFFFF'FFFFFFFF;

		mocks::MockMemoryStream stream("", buffer);

		// Act:
		pCacheStorage->loadAll(stream, 2);

		// Assert:
		auto pView = adapter.createView();
		AssertView<test::SimpleCacheView>(pView, 3);
	}

	// endregion

	// region general cache synchronization tests

	namespace {
		class SubCachePluginAdapterProxy {
		public:
			SubCachePluginAdapterProxy() : m_cache(CreateSimpleCacheWithValue(0))
			{}

		public:
			auto createView() const {
				return ViewProxy<const SubCacheView, test::SimpleCacheView>(m_cache.createView());
			}

			auto createDelta() {
				return ViewProxy<SubCacheView, test::SimpleCacheDelta>(m_cache.createDelta());
			}

			auto createDetachedDelta() const {
				return DetachedDeltaProxy(m_cache.createDetachedDelta());
			}

			void commit() {
				m_cache.commit();
			}

		private:
			template<typename TView, typename TRawView>
			class ViewProxy {
			public:
				explicit ViewProxy(std::unique_ptr<TView>&& pView) : m_pView(std::move(pView))
				{}

			public:
				const auto& operator*() const {
					return *static_cast<const TRawView*>(m_pView->get());
				}

				auto* operator->() {
					return static_cast<TRawView*>(m_pView->get());
				}

				explicit operator bool() const {
					return !!m_pView;
				}

			private:
				std::unique_ptr<TView> m_pView;
			};

			class DetachedDeltaProxy {
			public:
				explicit DetachedDeltaProxy(std::unique_ptr<DetachedSubCacheView>&& pView) : m_pView(std::move(pView))
				{}

			public:
				auto lock() {
					return ViewProxy<SubCacheView, test::SimpleCacheDelta>(m_pView->lock());
				}

			private:
				std::unique_ptr<DetachedSubCacheView> m_pView;
			};

		private:
			SimpleCachePluginAdapter m_cache;
		};

		struct SubCachePluginAdapterTraits {
		public:
			using CacheType = SubCachePluginAdapterProxy;

		public:
			static size_t MakeId(uint8_t id) {
				return id;
			}

			static size_t CreateWithId(uint8_t id) {
				return id;
			}
		};
	}

	DEFINE_CACHE_SYNC_TESTS(SubCachePluginAdapterTraits,)

	// endregion
}}
