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
		template<typename TViewExtension, typename TDeltaExtension>
		using SimpleCacheT = test::SimpleCacheT<3, TViewExtension, TDeltaExtension>;

		template<typename TViewExtension, typename TDeltaExtension>
		using SimpleCachePluginAdapterT = SubCachePluginAdapter<
			SimpleCacheT<TViewExtension, TDeltaExtension>,
			test::SimpleCacheExtensionStorageTraits<TViewExtension, TDeltaExtension>>;

		using SimpleCache = SimpleCacheT<test::SimpleCacheDefaultViewExtension, test::SimpleCacheDefaultDeltaExtension>;
		using SimpleCachePluginAdapter = SimpleCachePluginAdapterT<
			test::SimpleCacheDefaultViewExtension,
			test::SimpleCacheDefaultDeltaExtension>;

		template<typename TViewExtension, typename TDeltaExtension>
		std::unique_ptr<SimpleCacheT<TViewExtension, TDeltaExtension>> CreateSimpleCacheWithValueT(
				size_t value,
				test::SimpleCacheViewMode mode) {
			auto pCache = std::make_unique<SimpleCacheT<TViewExtension, TDeltaExtension>>(mode);
			auto delta = pCache->createDelta();
			for (auto i = 0u; i < value; ++i)
				delta->increment();

			pCache->commit();
			return pCache;
		}

		std::unique_ptr<SimpleCache> CreateSimpleCacheWithValue(
				size_t value,
				test::SimpleCacheViewMode mode = test::SimpleCacheViewMode::Iterable) {
			return CreateSimpleCacheWithValueT<test::SimpleCacheDefaultViewExtension, test::SimpleCacheDefaultDeltaExtension>(value, mode);
		}

		template<typename TRawView>
		void AssertConstSubCacheView(const SubCacheView& view, size_t expectedValue, SubCacheViewType expectedViewType) {
			// Assert: const raw view is correct
			ASSERT_TRUE(!!view.get());
			EXPECT_EQ(expectedValue, static_cast<const TRawView*>(view.get())->id());

			// - read only view is correct
			using ReadOnlyViewType = test::SimpleCacheReadOnlyType<
				test::SimpleCacheDefaultViewExtension,
				test::SimpleCacheDefaultDeltaExtension>;
			const auto* pReadOnlyView = static_cast<const ReadOnlyViewType*>(view.asReadOnly());
			ASSERT_TRUE(!!pReadOnlyView);
			EXPECT_EQ(expectedValue, pReadOnlyView->size());

			// - id is correct (name is truncated to 16 characters)
			EXPECT_EQ("SimpleCache (id ", std::string(view.id().CacheName.cbegin(), view.id().CacheName.cend()));
			EXPECT_EQ(3u, view.id().CacheId);
			EXPECT_EQ(expectedViewType, view.id().ViewType);
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
		void AssertView(const std::unique_ptr<const SubCacheView>& pView, size_t expectedValue, SubCacheViewType expectedViewType) {
			// Assert: a valid view was returned
			ASSERT_TRUE(!!pView);
			AssertConstSubCacheView<TRawView>(*pView, expectedValue, expectedViewType);
		}

		template<typename TRawView>
		void AssertView(const std::unique_ptr<SubCacheView>& pView, size_t expectedValue, SubCacheViewType expectedViewType) {
			// Assert: a valid view was returned
			ASSERT_TRUE(!!pView);
			AssertConstSubCacheView<TRawView>(*pView, expectedValue, expectedViewType);
			AssertNonConstSubCacheView<TRawView>(*pView, expectedValue);
		}
	}

	// region constructor / simple properties

	TEST(TEST_CLASS, CanAccessRawCachePointer) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act:
		const auto* pCache = static_cast<const SimpleCache*>(adapter.get());

		// Assert:
		ASSERT_TRUE(!!pCache);
		EXPECT_EQ(5u, pCache->createView()->id());
	}

	TEST(TEST_CLASS, CanAccessTypedCacheReference) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act + Assert:
		EXPECT_EQ(adapter.get(), &adapter.cache());
	}

	TEST(TEST_CLASS, CanAccessCacheName) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act:
		auto name = adapter.name();

		// Assert:
		EXPECT_EQ("SimpleCache (id = 3)", name);
	}

	TEST(TEST_CLASS, CanAccessCacheId) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act:
		auto id = adapter.id();

		// Assert:
		EXPECT_EQ(3u, id);
	}

	// endregion

	// region createView / createDelta

	TEST(TEST_CLASS, CanAccessView) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act:
		auto pView = adapter.createView();

		// Assert:
		AssertView<test::SimpleCacheView>(pView, 5, SubCacheViewType::View);
	}

	TEST(TEST_CLASS, CanAccessDelta) {
		// Arrange:
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));

		// Act:
		auto pDelta = adapter.createDelta();

		// Assert:
		AssertView<test::SimpleCacheDelta>(pDelta, 5, SubCacheViewType::Delta);
	}

	// endregion

	// region merkleRoot utils

	namespace {
		template<typename TAction>
		void RunTestForMerkleRootSupportedButDisabled(TAction action) {
			// Arrange:
			auto pCache = CreateSimpleCacheWithValue(5);

			SimpleCachePluginAdapter adapter(std::move(pCache));
			auto pView = adapter.createDelta();

			// Act + Assert:
			action(*pView);
		}

		template<typename TAction>
		void RunTestForMerkleRootSupportedAndEnabled(TAction action) {
			// Arrange:
			auto pCache = CreateSimpleCacheWithValue(5, test::SimpleCacheViewMode::Merkle_Root);
			auto expectedMerkleRoot = pCache->createView()->tryGetMerkleRoot().first;

			SimpleCachePluginAdapter adapter(std::move(pCache));
			auto pView = adapter.createDelta();

			// Act + Assert:
			action(*pView, expectedMerkleRoot);
		}

		template<typename TAction>
		void RunTestForMerkleRootSupportedAndEnabledView(TAction action) {
			// Arrange:
			auto pCache = CreateSimpleCacheWithValue(5, test::SimpleCacheViewMode::Merkle_Root);
			auto expectedMerkleRoot = pCache->createView()->tryGetMerkleRoot().first;

			SimpleCachePluginAdapter adapter(std::move(pCache));
			auto pView = adapter.createView();

			// Act + Assert:
			action(*pView, expectedMerkleRoot);
		}

		template<typename TAction>
		void RunTestForMerkleRootNotSupported(TAction action) {
			// Arrange:
			using CacheViewExtension = test::SimpleCacheDisabledMerkleRootViewExtension;
			auto pCache = CreateSimpleCacheWithValueT<CacheViewExtension, CacheViewExtension>(5, test::SimpleCacheViewMode::Merkle_Root);

			SimpleCachePluginAdapterT<CacheViewExtension, CacheViewExtension> adapter(std::move(pCache));
			auto pView = adapter.createDelta();

			// Act + Assert:
			action(*pView);
		}
	}

	// endregion

	// region merkleRoot - supportsMerkleRoot

	TEST(TEST_CLASS, SupportsMerkleRootReturnsFalseWhenMerkleRootIsSupportedButDisabled) {
		// Arrange:
		RunTestForMerkleRootSupportedButDisabled([](const auto& view) {
			// Act + Assert:
			EXPECT_FALSE(view.supportsMerkleRoot());
		});
	}

	TEST(TEST_CLASS, SupportsMerkleRootReturnsTrueWhenMerkleRootIsSupportedAndEnabled) {
		// Arrange:
		RunTestForMerkleRootSupportedAndEnabled([](const auto& view, const auto&) {
			// Act + Assert:
			EXPECT_TRUE(view.supportsMerkleRoot());
		});
	}

	TEST(TEST_CLASS, SupportsMerkleRootReturnsFalseWhenMerkleRootIsNotSupported) {
		// Arrange:
		RunTestForMerkleRootNotSupported([](const auto& view) {
			// Act + Assert:
			EXPECT_FALSE(view.supportsMerkleRoot());
		});
	}

	// endregion

	// region merkleRoot - tryGetMerkleRoot

	TEST(TEST_CLASS, CannotAccessMerkleRootWhenSupportedButDisabled) {
		// Arrange:
		RunTestForMerkleRootSupportedButDisabled([](const auto& view) {
			// Act:
			Hash256 merkleRoot;
			auto result = view.tryGetMerkleRoot(merkleRoot);

			// Assert:
			EXPECT_FALSE(result);
		});
	}

	TEST(TEST_CLASS, CanAccessMerkleRootWhenSupportedAndEnabled) {
		// Arrange:
		RunTestForMerkleRootSupportedAndEnabled([](const auto& view, const auto& expectedMerkleRoot) {
			// Act:
			Hash256 merkleRoot;
			auto result = view.tryGetMerkleRoot(merkleRoot);

			// Assert:
			EXPECT_TRUE(result);
			EXPECT_EQ(expectedMerkleRoot, merkleRoot);
		});
	}

	TEST(TEST_CLASS, CannotAccessMerkleRootWhenUnsupported) {
		// Arrange:
		RunTestForMerkleRootNotSupported([](const auto& view) {
			// Act:
			Hash256 merkleRoot;
			auto result = view.tryGetMerkleRoot(merkleRoot);

			// Assert:
			EXPECT_FALSE(result);
		});
	}

	// endregion

	// region merkleRoot - trySetMerkleRoot

	TEST(TEST_CLASS, CannotSetMerkleRootWhenSupportedButDisabled) {
		// Arrange:
		RunTestForMerkleRootSupportedButDisabled([](auto& view) {
			// Act:
			auto result = view.trySetMerkleRoot(Hash256());

			// Assert:
			EXPECT_FALSE(result);
		});
	}

	TEST(TEST_CLASS, CanSetMerkleRootWhenSupportedAndEnabledAndDelta) {
		// Arrange:
		RunTestForMerkleRootSupportedAndEnabled([](auto& view, const auto&) {
			// Act:
			auto result = view.trySetMerkleRoot(Hash256());

			// Assert:
			EXPECT_TRUE(result);

			Hash256 merkleRoot;
			EXPECT_TRUE(view.tryGetMerkleRoot(merkleRoot));
			EXPECT_EQ(Hash256(), merkleRoot);
		});
	}

	TEST(TEST_CLASS, CannotSetMerkleRootWhenSupportedAndEnabledButView) {
		// Arrange:
		RunTestForMerkleRootSupportedAndEnabledView([](auto& view, const auto&) {
			// Act: even if const is improperly casted away, operation should fail on const view
			auto result = const_cast<SubCacheView&>(view).trySetMerkleRoot(Hash256());

			// Assert:
			EXPECT_FALSE(result);
		});
	}

	TEST(TEST_CLASS, CannotSetMerkleRootWhenUnsupported) {
		// Arrange:
		RunTestForMerkleRootNotSupported([](auto& view) {
			// Act:
			auto result = view.trySetMerkleRoot(Hash256());

			// Assert:
			EXPECT_FALSE(result);
		});
	}

	// endregion

	// region merkleRoot - updateMerkleRoot

	TEST(TEST_CLASS, CannotUpdateMerkleRootWhenSupportedButDisabled) {
		// Arrange:
		RunTestForMerkleRootSupportedButDisabled([](auto& view) {
			// Act:
			view.updateMerkleRoot(Height(3));

			// Assert:
			Hash256 merkleRoot;
			EXPECT_FALSE(view.tryGetMerkleRoot(merkleRoot));
		});
	}

	TEST(TEST_CLASS, CanUpdateMerkleRootWhenSupportedAndEnabledAndDelta) {
		// Arrange:
		RunTestForMerkleRootSupportedAndEnabled([](auto& view, const auto& expectedMerkleRoot) {
			auto expectedUpdatedMerkleRoot = expectedMerkleRoot;
			expectedUpdatedMerkleRoot[0] = 3;

			// Act:
			view.updateMerkleRoot(Height(3));

			// Assert:
			Hash256 merkleRoot;
			EXPECT_TRUE(view.tryGetMerkleRoot(merkleRoot));
			EXPECT_EQ(expectedUpdatedMerkleRoot, merkleRoot);
		});
	}

	TEST(TEST_CLASS, CannotUpdateMerkleRootWhenSupportedAndEnabledButView) {
		// Arrange:
		RunTestForMerkleRootSupportedAndEnabledView([](auto& view, const auto& expectedMerkleRoot) {
			// Act: even if const is improperly casted away, operation should fail on const view
			const_cast<SubCacheView&>(view).updateMerkleRoot(Height(3));

			// Assert:
			Hash256 merkleRoot;
			EXPECT_TRUE(view.tryGetMerkleRoot(merkleRoot));
			EXPECT_EQ(expectedMerkleRoot, merkleRoot);
		});
	}

	TEST(TEST_CLASS, CannotUpdateMerkleRootWhenUnsupported) {
		// Arrange:
		RunTestForMerkleRootNotSupported([](auto& view) {
			// Act:
			view.updateMerkleRoot(Height(3));

			// Assert:
			Hash256 merkleRoot;
			EXPECT_FALSE(view.tryGetMerkleRoot(merkleRoot));
		});
	}

	// endregion

	// region prune

	TEST(TEST_CLASS, CanPruneWhenSupportedAndDelta) {
		// Arrange:
		RunTestForMerkleRootSupportedAndEnabled([](auto& view, const auto& expectedMerkleRoot) {
			auto expectedUpdatedMerkleRoot = expectedMerkleRoot;
			expectedUpdatedMerkleRoot[1] = 3;

			// Act:
			view.prune(Height(3));

			// Assert:
			Hash256 merkleRoot;
			EXPECT_TRUE(view.tryGetMerkleRoot(merkleRoot));
			EXPECT_EQ(expectedUpdatedMerkleRoot, merkleRoot);
		});
	}

	TEST(TEST_CLASS, CannotPruneWhenSupportedButView) {
		// Arrange:
		RunTestForMerkleRootSupportedAndEnabledView([](auto& view, const auto& expectedMerkleRoot) {
			// Act: even if const is improperly casted away, operation should fail on const view
			const_cast<SubCacheView&>(view).prune(Height(3));

			// Assert:
			Hash256 merkleRoot;
			EXPECT_TRUE(view.tryGetMerkleRoot(merkleRoot));
			EXPECT_EQ(expectedMerkleRoot, merkleRoot);
		});
	}

	TEST(TEST_CLASS, CannotPruneWhenUnsupported) {
		// Arrange:
		RunTestForMerkleRootNotSupported([](auto& view) {
			// Act:
			view.prune(Height(3));

			// Assert:
			Hash256 merkleRoot;
			EXPECT_FALSE(view.tryGetMerkleRoot(merkleRoot));
		});
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
		auto pDelta = pDetachedDelta->tryLock();

		// Assert:
		AssertView<test::SimpleCacheDelta>(pDelta, 5, SubCacheViewType::DetachedDelta);
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
		auto pDelta = pDetachedDelta->tryLock();

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
		AssertView<test::SimpleCacheView>(pView, 5, SubCacheViewType::View);
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
		AssertView<test::SimpleCacheView>(pView, 6, SubCacheViewType::View);
	}

	// endregion

	// region createStorage

	namespace {
		constexpr auto Xor_Operand = 0xFFFFFFFF'FFFFFFFFu;
	}

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
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(0, test::SimpleCacheViewMode::Basic));

		// Act:
		auto pCacheStorage = adapter.createStorage();

		// Assert:
		ASSERT_FALSE(!!pCacheStorage);
	}

	TEST(TEST_CLASS, CanSerializeCacheToStorage) {
		// Arrange:
		auto pAdapter = std::make_unique<SimpleCachePluginAdapter>(CreateSimpleCacheWithValue(5));
		auto pCacheStorage = pAdapter->createStorage();
		ASSERT_TRUE(!!pCacheStorage);

		std::vector<std::unique_ptr<SubCachePlugin>> subCaches(4);
		subCaches[3] = std::move(pAdapter);
		CatapultCache cache(std::move(subCaches));
		auto cacheView = cache.createView();

		// Act:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);
		pCacheStorage->saveAll(cacheView, stream);

		// Assert:
		ASSERT_EQ(6 * sizeof(uint64_t), buffer.size());

		const auto* pData64 = reinterpret_cast<const uint64_t*>(buffer.data());
		EXPECT_EQ(5u, pData64[0]); // size;

		for (auto i = 1u; i <= 5; ++i)
			EXPECT_EQ(i ^ Xor_Operand, pData64[i]) << "value at " << i;
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
			pData64[i] = i ^ Xor_Operand;

		// Act:
		mocks::MockMemoryStream stream(buffer);
		pCacheStorage->loadAll(stream, 2);

		// Assert:
		auto pView = adapter.createView();
		AssertView<test::SimpleCacheView>(pView, 3, SubCacheViewType::View);
	}

	// endregion

	// region createChangesStorage

	namespace {
		std::unordered_set<uint64_t> XorAll(const std::unordered_set<uint64_t>& values) {
			std::unordered_set<uint64_t> xoredValues;
			for (auto value : values)
				xoredValues.insert(value ^ Xor_Operand);

			return xoredValues;
		}
	}

	TEST(TEST_CLASS, CanSerializeCacheChangesToStream) {
		// Arrange: create cache changes storage
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));
		auto pCacheChangesStorage = adapter.createChangesStorage();
		ASSERT_TRUE(!!pCacheChangesStorage);

		// - create cache changes (simple cache id is 3)
		auto pSubCacheChanges = std::make_unique<MemoryCacheChangesT<uint64_t>>();
		pSubCacheChanges->Added = { 1, 16 };
		pSubCacheChanges->Removed = { 9, 25, 36 };
		pSubCacheChanges->Copied = { 4 };

		CacheChanges::MemoryCacheChangesContainer cacheChangesContainer;
		cacheChangesContainer.emplace_back(nullptr);
		cacheChangesContainer.emplace_back(nullptr);
		cacheChangesContainer.emplace_back(nullptr);
		cacheChangesContainer.emplace_back(std::move(pSubCacheChanges));
		CacheChanges changes(std::move(cacheChangesContainer));

		// Act: write changes to buffer
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);
		pCacheChangesStorage->saveAll(changes, stream);

		ASSERT_EQ(9 * sizeof(uint64_t), buffer.size());
		const auto* pData64 = reinterpret_cast<const uint64_t*>(buffer.data());

		// - header
		EXPECT_EQ(2u, pData64[0]);
		EXPECT_EQ(3u, pData64[1]);
		EXPECT_EQ(1u, pData64[2]);

		// values
		EXPECT_EQ(XorAll({ 1, 16 }), std::unordered_set<uint64_t>({ pData64[3], pData64[4] }));
		EXPECT_EQ(XorAll({ 9, 25, 36 }), std::unordered_set<uint64_t>({ pData64[5], pData64[6], pData64[7] }));
		EXPECT_EQ(XorAll({ 4 }), std::unordered_set<uint64_t>({ pData64[8] }));
	}

	TEST(TEST_CLASS, CanDeserializeCacheChangesFromStream) {
		// Arrange: create cache changes storage
		SimpleCachePluginAdapter adapter(CreateSimpleCacheWithValue(5));
		auto pCacheChangesStorage = adapter.createChangesStorage();
		ASSERT_TRUE(!!pCacheChangesStorage);

		// - prepare the input
		std::vector<uint8_t> buffer(9 * sizeof(uint64_t));
		auto* pData64 = reinterpret_cast<uint64_t*>(buffer.data());
		for (auto value : std::initializer_list<uint64_t>{ 2, 3, 1 })
			*pData64++ = value;

		for (auto value : std::initializer_list<uint64_t>{ 1, 16, 9, 25, 36, 4 })
			*pData64++ = value ^ Xor_Operand;

		// Act:
		mocks::MockMemoryStream stream(buffer);
		auto pChangesVoid = pCacheChangesStorage->loadAll(stream);
		const auto& changes = static_cast<const MemoryCacheChangesT<uint64_t>&>(*pChangesVoid);

		// Assert:
		EXPECT_EQ(2u, changes.Added.size());
		EXPECT_EQ(3u, changes.Removed.size());
		EXPECT_EQ(1u, changes.Copied.size());

		EXPECT_EQ(std::vector<uint64_t>({ 1, 16 }), changes.Added);
		EXPECT_EQ(std::vector<uint64_t>({ 9, 25, 36 }), changes.Removed);
		EXPECT_EQ(std::vector<uint64_t>({ 4 }), changes.Copied);
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
				auto tryLock() {
					return ViewProxy<SubCacheView, test::SimpleCacheDelta>(m_pView->tryLock());
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
