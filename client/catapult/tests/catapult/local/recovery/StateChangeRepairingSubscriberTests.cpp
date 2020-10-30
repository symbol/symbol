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

#include "catapult/local/recovery/StateChangeRepairingSubscriber.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/subscribers/StateChangeInfo.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/cache/SimpleCache.h"

namespace catapult { namespace local {

#define TEST_CLASS StateChangeRepairingSubscriberTests

	namespace {
		// region mock storage

		template<typename TViewExtension, typename TDeltaExtension>
		struct MockSimpleCacheExtensionStorageTraits {
			using SourceType = test::SimpleCacheViewExtension<TViewExtension, TDeltaExtension>;
			using DestinationType = test::SimpleCacheDeltaExtension<TViewExtension, TDeltaExtension>;

			[[noreturn]]
			static void Save(uint64_t, io::OutputStream&) {
				CATAPULT_THROW_RUNTIME_ERROR("Save() unsupported in mock storage");
			}

			[[noreturn]]
			static uint64_t Load(io::InputStream&) {
				CATAPULT_THROW_RUNTIME_ERROR("Load() unsupported in mock storage");
			}

			static void LoadInto(uint64_t, DestinationType&)
			{}

			// let every purge operation modify cache delta id
			static void Purge(uint64_t value, DestinationType& cacheDelta) {
				cacheDelta.insert(cacheDelta.id() + value);
			}
		};

		using SimpleCacheStorageTraits = MockSimpleCacheExtensionStorageTraits<
			test::SimpleCacheDefaultViewExtension,
			test::SimpleCacheDefaultDeltaExtension>;

		// endregion

		auto CreateCatapultCache() {
			std::vector<std::unique_ptr<cache::SubCachePlugin>> subCaches(3);
			subCaches[1] = test::MakeSubCachePlugin<test::SimpleCacheT<1>, SimpleCacheStorageTraits>();
			subCaches[2] = test::MakeSubCachePlugin<test::SimpleCacheT<2>, SimpleCacheStorageTraits>();
			return cache::CatapultCache(std::move(subCaches));
		}

		using CacheElements = std::vector<uint64_t>;
		auto CreateMemoryCacheChanges(const CacheElements& added, const CacheElements& removed, const CacheElements& copied) {
			auto pChanges = std::make_unique<cache::MemoryCacheChangesT<uint64_t>>();
			pChanges->Added = added;
			pChanges->Removed = removed;
			pChanges->Copied = copied;
			return pChanges;
		}
	}

	TEST(TEST_CLASS, NotifyScoreChangeUpdatesLocalNodeChainScore) {
		// Arrange:
		auto cache = CreateCatapultCache();
		extensions::LocalNodeChainScore chainScore(model::ChainScore(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6));
		auto pSubscriber = CreateStateChangeRepairingSubscriber(cache, chainScore);

		// Act:
		pSubscriber->notifyScoreChange(model::ChainScore(123, 435));

		// Assert:
		EXPECT_EQ(model::ChainScore(123, 435), chainScore.get());
	}

	TEST(TEST_CLASS, NotifyStateChangeUpdatesCacheStorage) {
		// Arrange:
		auto cache = CreateCatapultCache();
		extensions::LocalNodeChainScore chainScore(model::ChainScore(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6));
		auto pSubscriber = CreateStateChangeRepairingSubscriber(cache, chainScore);

		cache::CacheChanges::MemoryCacheChangesContainer cacheChangesContainer;
		cacheChangesContainer.emplace_back(nullptr);
		cacheChangesContainer.emplace_back(CreateMemoryCacheChanges({ 21, 34 }, { 13 }, { 3, 5, 8 }));
		cacheChangesContainer.emplace_back(CreateMemoryCacheChanges({ 50, 60 }, { 40 }, { 10, 20, 30 }));
		cache::CacheChanges cacheChanges(std::move(cacheChangesContainer));
		subscribers::StateChangeInfo stateChangeInfo(std::move(cacheChanges), model::ChainScore::Delta(435), Height(234));

		// Act:
		pSubscriber->notifyStateChange(stateChangeInfo);

		// Assert: caches are updated, chain score is untouched
		auto cacheView = cache.createView();
		EXPECT_EQ(model::ChainScore(0x8FDE'4267'9C23'D678, 0x7A6B'3481'0235'43B6), chainScore.get());
		EXPECT_EQ(Height(234), cacheView.height());

		const auto& subView1 = cacheView.sub<test::SimpleCacheT<1>>();
		const auto& subView2 = cacheView.sub<test::SimpleCacheT<2>>();
		EXPECT_EQ(3u + 5 + 8 + 13 + 21 + 34, subView1.id());
		EXPECT_EQ(10u + 20 + 30 + 40 + 50 + 60, subView2.id());
	}
}}
