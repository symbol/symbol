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

#include "harvesting/src/DelegatePrioritizationPolicy.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS DelegatePrioritizationPolicyTests

	// region DelegatePrioritizationPolicy

	TEST(TEST_CLASS, CanParseValidDelegatePrioritizationPolicy) {
		test::AssertParse("Age", DelegatePrioritizationPolicy::Age, TryParseValue);
		test::AssertParse("Importance", DelegatePrioritizationPolicy::Importance, TryParseValue);
	}

	// endregion

	// region CreateDelegatePrioritizer

	namespace {
		auto CreateCatapultCacheWithImportantAccount(const Key& importantAccountPublicKey, Height cacheHeight = Height(100)) {
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			blockChainConfig.ImportanceGrouping = 1;
			auto cache = test::CreateEmptyCatapultCache(blockChainConfig);

			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
			accountStateCacheDelta.addAccount(importantAccountPublicKey, Height(100));
			accountStateCacheDelta.find(importantAccountPublicKey).get().ImportanceSnapshots.set(
					Importance(1234),
					model::ImportanceHeight(100));
			cache.commit(cacheHeight);

			return cache;
		}
	}

	TEST(TEST_CLASS, CanCreateDelegatePrioritizerForAgePolicy) {
		// Arrange:
		auto primaryAccountPublicKey = test::GenerateRandomByteArray<Key>();
		auto importantAccountPublicKey = test::GenerateRandomByteArray<Key>();
		auto cache = CreateCatapultCacheWithImportantAccount(importantAccountPublicKey);

		// Act:
		auto prioritizer = CreateDelegatePrioritizer(DelegatePrioritizationPolicy::Age, cache, primaryAccountPublicKey);

		// Assert:
		EXPECT_EQ(0u, prioritizer(primaryAccountPublicKey));
		EXPECT_EQ(0u, prioritizer(importantAccountPublicKey));
		EXPECT_EQ(0u, prioritizer(test::GenerateRandomByteArray<Key>()));
	}

	TEST(TEST_CLASS, CanCreateDelegatePrioritizerForImportancePolicy) {
		// Arrange:
		auto primaryAccountPublicKey = test::GenerateRandomByteArray<Key>();
		auto importantAccountPublicKey = test::GenerateRandomByteArray<Key>();
		auto cache = CreateCatapultCacheWithImportantAccount(importantAccountPublicKey);

		// Act:
		auto prioritizer = CreateDelegatePrioritizer(DelegatePrioritizationPolicy::Importance, cache, primaryAccountPublicKey);

		// Assert:
		EXPECT_EQ(std::numeric_limits<uint64_t>::max(), prioritizer(primaryAccountPublicKey));
		EXPECT_EQ(1234u, prioritizer(importantAccountPublicKey));
		EXPECT_EQ(0u, prioritizer(test::GenerateRandomByteArray<Key>()));
	}

	TEST(TEST_CLASS, CanCreateDelegatePrioritizerForImportancePolicy_RespectsHeight) {
		// Arrange:
		for (auto height : { Height(99), Height(101) }) {
			auto primaryAccountPublicKey = test::GenerateRandomByteArray<Key>();
			auto importantAccountPublicKey = test::GenerateRandomByteArray<Key>();
			auto cache = CreateCatapultCacheWithImportantAccount(importantAccountPublicKey, height);

			// Act:
			auto prioritizer = CreateDelegatePrioritizer(DelegatePrioritizationPolicy::Importance, cache, primaryAccountPublicKey);

			// Assert:
			EXPECT_EQ(0u, prioritizer(importantAccountPublicKey)) << height;
		}
	}

	TEST(TEST_CLASS, CannotCreateDelegatePrioritizerForUnknownPolicy) {
		// Arrange:
		auto primaryAccountPublicKey = test::GenerateRandomByteArray<Key>();
		auto cache = CreateCatapultCacheWithImportantAccount(test::GenerateRandomByteArray<Key>());

		// Act + Assert:
		EXPECT_THROW(
				CreateDelegatePrioritizer(static_cast<DelegatePrioritizationPolicy>(100), cache, primaryAccountPublicKey),
				catapult_invalid_argument);
	}

	// endregion
}}
