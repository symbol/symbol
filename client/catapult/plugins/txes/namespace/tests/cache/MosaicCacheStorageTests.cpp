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

#include "src/cache/MosaicCacheStorage.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicHistoryLoadTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS MosaicCacheStorageTests

	using test::AssertMosaicEntry;

	namespace {
		void LoadInto(io::InputStream& inputStream, MosaicCacheDelta& delta) {
			return MosaicCacheStorage::LoadInto(MosaicCacheStorage::Load(inputStream), delta);
		}

		struct LoadTraits {
			using MosaicHistoryHeader = test::MosaicHistoryHeader;

			static MosaicHistoryHeader CreateHistoryHeader(NamespaceId namespaceId, MosaicId id, uint64_t depth) {
				return { depth, namespaceId, id };
			}

			static void AssertCannotLoad(io::InputStream& inputStream) {
				// Assert:
				MosaicCache cache(CacheConfiguration{});
				auto delta = cache.createDelta();
				EXPECT_THROW(LoadInto(inputStream, *delta), catapult_runtime_error);
			}

			static void AssertCanLoadWithDepthOne(io::InputStream& inputStream, const Key& owner) {
				// Act:
				MosaicCache cache(CacheConfiguration{});
				auto delta = cache.createDelta();
				LoadInto(inputStream, *delta);
				cache.commit();

				// Assert:
				auto view = cache.createView();
				test::AssertCacheSizes(*view, 1, 1);

				ASSERT_TRUE(view->contains(MosaicId(123)));
				AssertMosaicEntry(view->find(MosaicId(123)).get(), NamespaceId(987), Height(222), owner, { { 9, 8, 7 } }, Amount(786));
			}

			static void AssertCanLoadWithDepthGreaterThanOne(io::InputStream& inputStream, const Key& owner1, const Key& owner2) {
				// Act:
				MosaicCache cache(CacheConfiguration{});
				auto delta = cache.createDelta();
				LoadInto(inputStream, *delta);
				cache.commit();

				// Assert:
				auto view = cache.createView();
				test::AssertCacheSizes(*view, 1, 3);

				ASSERT_TRUE(view->contains(MosaicId(123)));
				AssertMosaicEntry(view->find(MosaicId(123)).get(), NamespaceId(987), Height(456), owner1, { { 1, 2, 4 } }, Amount(645));

				// - check history (one back)
				delta->remove(MosaicId(123));
				test::AssertCacheSizes(*delta, 1, 2);
				AssertMosaicEntry(delta->find(MosaicId(123)).get(), NamespaceId(987), Height(321), owner2, { { 2, 5, 7 } }, Amount(999));

				// - check history (two back)
				delta->remove(MosaicId(123));
				test::AssertCacheSizes(*delta, 1, 1);
				AssertMosaicEntry(delta->find(MosaicId(123)).get(), NamespaceId(987), Height(222), owner1, { { 9, 8, 7 } }, Amount(786));
			}
		};
	}

	DEFINE_MOSAIC_HISTORY_LOAD_TESTS(LoadTraits,)
}}
