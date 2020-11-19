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

#include "catapult/cache/CatapultCacheBuilder.h"
#include "tests/test/cache/SimpleCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CatapultCacheBuilderTests

	namespace {
		template<size_t CacheId>
		class CustomSubCachePluginAdapter : public SubCachePluginAdapter<test::SimpleCacheT<CacheId>, test::SimpleCacheStorageTraits> {
		public:
			using SubCachePluginAdapter<test::SimpleCacheT<CacheId>, test::SimpleCacheStorageTraits>::SubCachePluginAdapter;
		};

		template<size_t CacheId>
		void AddSubCacheWithId(CatapultCacheBuilder& builder) {
			builder.add<test::SimpleCacheStorageTraits>(std::make_unique<test::SimpleCacheT<CacheId>>());
		}

		template<size_t CacheId>
		void AddSubCachePluginWithId(CatapultCacheBuilder& builder) {
			builder.add(std::make_unique<CustomSubCachePluginAdapter<CacheId>>(std::make_unique<test::SimpleCacheT<CacheId>>()));
		}

		size_t GetNumSubCaches(const CatapultCache& cache) {
			return cache.storages().size();
		}
	}

	TEST(TEST_CLASS, CanCreateEmptyCatapultCache) {
		// Arrange:
		CatapultCacheBuilder builder;

		// Act:
		auto cache = builder.build();

		// Assert:
		EXPECT_EQ(0u, GetNumSubCaches(cache));
	}

	TEST(TEST_CLASS, CanCreateCatapultCacheWithSingleSubCache_DefaultPlugin) {
		// Arrange:
		CatapultCacheBuilder builder;
		AddSubCacheWithId<2>(builder);

		// Act:
		auto cache = builder.build();

		// Assert:
		EXPECT_EQ(1u, GetNumSubCaches(cache));
	}

	TEST(TEST_CLASS, CanCreateCatapultCacheWithSingleSubCache_CustomPlugin) {
		// Arrange:
		CatapultCacheBuilder builder;
		AddSubCachePluginWithId<2>(builder);

		// Act:
		auto cache = builder.build();

		// Assert:
		EXPECT_EQ(1u, GetNumSubCaches(cache));
	}

	TEST(TEST_CLASS, CanCreateCatapultCacheWithMultipleSubCaches) {
		// Arrange:
		CatapultCacheBuilder builder;
		AddSubCacheWithId<2>(builder);
		AddSubCachePluginWithId<6>(builder);
		AddSubCacheWithId<4>(builder);
		AddSubCachePluginWithId<8>(builder);

		// Act:
		auto cache = builder.build();

		// Assert:
		EXPECT_EQ(4u, GetNumSubCaches(cache));
	}

	TEST(TEST_CLASS, CannotAddMultipleSubCachesWithSameId) {
		// Arrange:
		CatapultCacheBuilder builder;
		AddSubCacheWithId<2>(builder);
		AddSubCacheWithId<6>(builder);
		AddSubCacheWithId<4>(builder);

		// Act + Assert:
		EXPECT_THROW(AddSubCacheWithId<6>(builder), catapult_invalid_argument);
		EXPECT_THROW(AddSubCachePluginWithId<6>(builder), catapult_invalid_argument);
	}
}}
