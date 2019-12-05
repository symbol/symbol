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

#include "catapult/cache/CacheChanges.h"
#include "catapult/cache/CatapultCacheBuilder.h"
#include "tests/catapult/cache/test/ByteVectorCacheChanges.h"
#include "tests/catapult/cache/test/DeltasAwareCache.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS CacheChangesTests

	// region test utils

	namespace {
		using VectorCacheDeltas = test::ByteVectorCacheDeltas;
		using VectorSingleCacheChanges = SingleCacheChangesT<deltaset::DeltaElementsMixin<VectorCacheDeltas>, std::vector<uint8_t>>;
		using VectorMemoryCacheChanges = test::ByteVectorCacheChanges;

		void SeedRandom(VectorCacheDeltas& deltas, uint8_t numAdded, uint8_t numModified, uint8_t numRemoved) {
			for (uint8_t i = 0; i < numAdded; ++i)
				deltas.Added.emplace(i, test::GenerateRandomVector(12 + i));

			for (uint8_t i = 0; i < numModified; ++i)
				deltas.Copied.emplace(i, test::GenerateRandomVector(10 + i));

			for (uint8_t i = 0; i < numRemoved; ++i)
				deltas.Removed.emplace(i, test::GenerateRandomVector(14 + i));
		}

		void SeedRandom(VectorMemoryCacheChanges& changes, uint8_t numAdded, uint8_t numModified, uint8_t numRemoved) {
			for (uint8_t i = 0; i < numAdded; ++i)
				changes.Added.emplace_back(test::GenerateRandomVector(12 + i));

			for (uint8_t i = 0; i < numModified; ++i)
				changes.Copied.emplace_back(test::GenerateRandomVector(10 + i));

			for (uint8_t i = 0; i < numRemoved; ++i)
				changes.Removed.emplace_back(test::GenerateRandomVector(14 + i));
		}
	}

	// endregion

	// region SingleCacheChanges - cache delta

	TEST(TEST_CLASS, SingleCacheChangesCanDelegateToCacheDelta_AddedElements) {
		// Arrange:
		VectorCacheDeltas deltas;
		SeedRandom(deltas, 2, 3, 4);
		deltaset::DeltaElementsMixin<VectorCacheDeltas> cacheDelta(deltas);

		// Act:
		VectorSingleCacheChanges cacheChanges(cacheDelta);
		auto result = cacheChanges.addedElements();

		// Assert:
		EXPECT_EQ(2u, result.size());
		EXPECT_EQ(cacheDelta.addedElements(), result);
	}

	TEST(TEST_CLASS, SingleCacheChangesCanDelegateToCacheDelta_ModifiedElements) {
		// Arrange:
		VectorCacheDeltas deltas;
		SeedRandom(deltas, 2, 3, 4);
		deltaset::DeltaElementsMixin<VectorCacheDeltas> cacheDelta(deltas);

		// Act:
		VectorSingleCacheChanges cacheChanges(cacheDelta);
		auto result = cacheChanges.modifiedElements();

		// Assert:
		EXPECT_EQ(3u, result.size());
		EXPECT_EQ(cacheDelta.modifiedElements(), result);
	}

	TEST(TEST_CLASS, SingleCacheChangesCanDelegateToCacheDelta_RemovedElements) {
		// Arrange:
		VectorCacheDeltas deltas;
		SeedRandom(deltas, 2, 3, 4);
		deltaset::DeltaElementsMixin<VectorCacheDeltas> cacheDelta(deltas);

		// Act:
		VectorSingleCacheChanges cacheChanges(cacheDelta);
		auto result = cacheChanges.removedElements();

		// Assert:
		EXPECT_EQ(4u, result.size());
		EXPECT_EQ(cacheDelta.removedElements(), result);
	}

	// endregion

	// region SingleCacheChanges - memory cache changes

	TEST(TEST_CLASS, SingleCacheChangesCanDelegateToMemoryCacheChanges_AddedElements) {
		// Arrange:
		VectorMemoryCacheChanges memoryCacheChanges;
		SeedRandom(memoryCacheChanges, 2, 3, 4);

		// Act:
		VectorSingleCacheChanges cacheChanges(memoryCacheChanges);
		auto result = cacheChanges.addedElements();

		// Assert:
		EXPECT_EQ(2u, result.size());
		decltype(result) expected{ &memoryCacheChanges.Added[0], &memoryCacheChanges.Added[1] };
		EXPECT_EQ(expected, result);
	}

	TEST(TEST_CLASS, SingleCacheChangesCanDelegateToMemoryCacheChanges_ModifiedElements) {
		// Arrange:
		VectorMemoryCacheChanges memoryCacheChanges;
		SeedRandom(memoryCacheChanges, 2, 3, 4);

		// Act:
		VectorSingleCacheChanges cacheChanges(memoryCacheChanges);
		auto result = cacheChanges.modifiedElements();

		// Assert:
		ASSERT_EQ(3u, result.size());
		decltype(result) expected{ &memoryCacheChanges.Copied[0], &memoryCacheChanges.Copied[1], &memoryCacheChanges.Copied[2] };
		EXPECT_EQ(expected, result);
	}

	TEST(TEST_CLASS, SingleCacheChangesCanDelegateToMemoryCacheChanges_RemovedElements) {
		// Arrange:
		VectorMemoryCacheChanges memoryCacheChanges;
		SeedRandom(memoryCacheChanges, 2, 3, 4);

		// Act:
		VectorSingleCacheChanges cacheChanges(memoryCacheChanges);
		auto result = cacheChanges.removedElements();

		// Assert:
		ASSERT_EQ(4u, result.size());
		decltype(result) expected{
			&memoryCacheChanges.Removed[0], &memoryCacheChanges.Removed[1], &memoryCacheChanges.Removed[2], &memoryCacheChanges.Removed[3]
		};
		EXPECT_EQ(expected, result);
	}

	// endregion

	// region SingleCacheChanges - ordering

	TEST(TEST_CLASS, SingleCacheChangesDoesNotPreserveOrderOfMapElements) {
		// Arrange:
		VectorCacheDeltas deltas;
		deltaset::DeltaElementsMixin<VectorCacheDeltas> cacheDelta(deltas);

		// Act:
		VectorSingleCacheChanges cacheChanges(cacheDelta);
		auto result = cacheChanges.addedElements();

		// Assert:
		EXPECT_FALSE(utils::traits::is_map_v<std::remove_reference_t<decltype(result)>>);
		EXPECT_FALSE(utils::traits::is_ordered_v<std::remove_reference_t<decltype(result)>>);
	}

	TEST(TEST_CLASS, SingleCacheChangesPreservesOrderOfOrderedSetElements) {
		// Arrange:
		using ByteSetCacheDeltas = test::DeltaElementsTestUtils::Wrapper<std::set<uint8_t>>;
		using ByteSetSingleCacheChanges = SingleCacheChangesT<deltaset::DeltaElementsMixin<ByteSetCacheDeltas>, uint8_t>;

		ByteSetCacheDeltas deltas;
		deltaset::DeltaElementsMixin<ByteSetCacheDeltas> cacheDelta(deltas);

		// Act:
		ByteSetSingleCacheChanges cacheChanges(cacheDelta);
		auto result = cacheChanges.addedElements();

		// Assert:
		EXPECT_FALSE(utils::traits::is_map_v<std::remove_reference_t<decltype(result)>>);
		EXPECT_TRUE(utils::traits::is_ordered_v<std::remove_reference_t<decltype(result)>>);
	}

	// endregion

	// region CacheChanges

	namespace {
		bool Contains(const std::unordered_set<const std::vector<uint8_t>*>& pointers, const std::vector<uint8_t>& value) {
			return std::any_of(pointers.cbegin(), pointers.cend(), [&value](const auto* pValue) {
				return value == *pValue;
			});
		}

		template<typename TDeltas, typename TUnwrap>
		void AssertCacheChanges(const CacheChanges& cacheChanges, const TDeltas& deltas1, const TDeltas& deltas2, TUnwrap unwrap) {
			// Act: access sub cache changes
			auto subCacheChanges0 = cacheChanges.sub<test::DeltasAwareCache<0>>();
			auto subCacheChanges2 = cacheChanges.sub<test::DeltasAwareCache<2>>();

			// Assert: check sizes
			EXPECT_EQ(1u, subCacheChanges0.addedElements().size());
			EXPECT_EQ(3u, subCacheChanges0.modifiedElements().size());
			EXPECT_EQ(2u, subCacheChanges0.removedElements().size());

			EXPECT_EQ(3u, subCacheChanges2.addedElements().size());
			EXPECT_EQ(2u, subCacheChanges2.modifiedElements().size());
			EXPECT_EQ(4u, subCacheChanges2.removedElements().size());

			// - check added as proxy
			auto i = 0u;
			for (const auto& value : deltas1.Added)
				EXPECT_TRUE(Contains(subCacheChanges0.addedElements(), unwrap(value))) << "at " << i;

			i = 0;
			for (const auto& value : deltas2.Added)
				EXPECT_TRUE(Contains(subCacheChanges2.addedElements(), unwrap(value))) << "at " << i;
		}
	}

	TEST(TEST_CLASS, CacheChangesCanDelegateToCacheDelta) {
		// Arrange: set up cache with two sub caches
		CatapultCacheBuilder builder;

		VectorCacheDeltas deltas1;
		SeedRandom(deltas1, 1, 3, 2);
		builder.add<test::DeltasAwareCacheStorageTraits>(std::make_unique<test::DeltasAwareCache<0>>(deltas1));

		VectorCacheDeltas deltas2;
		SeedRandom(deltas2, 3, 2, 4);
		builder.add<test::DeltasAwareCacheStorageTraits>(std::make_unique<test::DeltasAwareCache<2>>(deltas2));

		// - create cache changes
		auto cache = builder.build();
		auto cacheDelta = cache.createDelta();
		CacheChanges cacheChanges(cacheDelta);

		// Act + Assert:
		AssertCacheChanges<VectorCacheDeltas>(cacheChanges, deltas1, deltas2, [](const auto& pair) {
			return pair.second;
		});
	}

	TEST(TEST_CLASS, CacheChangesCanDelegateToMemoryCacheChanges) {
		// Arrange: set up memory cache changes vector
		VectorMemoryCacheChanges memoryCacheChanges1;
		SeedRandom(memoryCacheChanges1, 1, 3, 2);

		VectorMemoryCacheChanges memoryCacheChanges2;
		SeedRandom(memoryCacheChanges2, 3, 2, 4);

		// - create cache changes
		CacheChanges::MemoryCacheChangesContainer cacheChangesContainer;
		cacheChangesContainer.emplace_back(test::CopyByteVectorCacheChanges(memoryCacheChanges1));
		cacheChangesContainer.emplace_back(nullptr);
		cacheChangesContainer.emplace_back(test::CopyByteVectorCacheChanges(memoryCacheChanges2));
		CacheChanges cacheChanges(std::move(cacheChangesContainer));

		// Act + Assert:
		AssertCacheChanges(cacheChanges, memoryCacheChanges1, memoryCacheChanges2, [](const auto& value) {
			return value;
		});
	}

	// endregion
}}
