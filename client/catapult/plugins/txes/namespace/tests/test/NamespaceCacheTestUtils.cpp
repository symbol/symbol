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

#include "NamespaceCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		template<typename TCache>
		void AssertCacheSizesT(const TCache& cache, size_t size, size_t activeSize, size_t deepSize) {
			// Assert:
			EXPECT_EQ(size, cache.size());
			EXPECT_EQ(activeSize, cache.activeSize());
			EXPECT_EQ(deepSize, cache.deepSize());
		}
	}

	void AssertCacheSizes(const cache::NamespaceCacheView& cache, size_t size, size_t activeSize, size_t deepSize) {
		AssertCacheSizesT(cache, size, activeSize, deepSize);
	}

	void AssertCacheSizes(const cache::NamespaceCacheDelta& cache, size_t size, size_t activeSize, size_t deepSize) {
		AssertCacheSizesT(cache, size, activeSize, deepSize);
	}

	namespace {
		template<typename TCache>
		void AssertCacheContentsT(const TCache& cache, std::initializer_list<NamespaceId::ValueType> expectedIds) {
			// Assert:
			EXPECT_EQ(expectedIds.size(), cache.activeSize());
			for (auto id : expectedIds)
				EXPECT_TRUE(cache.contains(NamespaceId(id))) << "id " << id;
		}
	}

	void AssertCacheContents(const cache::NamespaceCache& cache, std::initializer_list<NamespaceId::ValueType> expectedIds) {
		auto view = cache.createView();
		AssertCacheContentsT(*view, expectedIds);
	}

	void AssertCacheContents(const cache::NamespaceCacheView& cache, std::initializer_list<NamespaceId::ValueType> expectedIds) {
		AssertCacheContentsT(cache, expectedIds);
	}

	void AssertCacheContents(const cache::NamespaceCacheDelta& cache, std::initializer_list<NamespaceId::ValueType> expectedIds) {
		AssertCacheContentsT(cache, expectedIds);
	}
}}
