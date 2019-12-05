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

#include "catapult/cache_core/ReadOnlyAccountStateCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS ReadOnlyAccountStateCacheTests

	// region cache properties

	namespace {
		constexpr auto Default_Cache_Options = test::CreateDefaultAccountStateCacheOptions();
	}

	TEST(TEST_CLASS, NetworkIdentifierIsExposed) {
		// Arrange:
		auto networkIdentifier = static_cast<model::NetworkIdentifier>(19);
		auto options = Default_Cache_Options;
		options.NetworkIdentifier = networkIdentifier;
		AccountStateCache originalCache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(networkIdentifier, ReadOnlyAccountStateCache(*originalCache.createView()).networkIdentifier());
		EXPECT_EQ(networkIdentifier, ReadOnlyAccountStateCache(*originalCache.createDelta()).networkIdentifier());
	}

	TEST(TEST_CLASS, ImportanceGroupingIsExposed) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.ImportanceGrouping = 123;
		AccountStateCache originalCache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(123u, ReadOnlyAccountStateCache(*originalCache.createView()).importanceGrouping());
		EXPECT_EQ(123u, ReadOnlyAccountStateCache(*originalCache.createDelta()).importanceGrouping());
	}

	TEST(TEST_CLASS, MinHarvesterBalanceIsExposed) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.MinHarvesterBalance = Amount(336644);
		AccountStateCache originalCache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(Amount(336644), ReadOnlyAccountStateCache(*originalCache.createView()).minHarvesterBalance());
		EXPECT_EQ(Amount(336644), ReadOnlyAccountStateCache(*originalCache.createDelta()).minHarvesterBalance());
	}

	TEST(TEST_CLASS, MaxHarvesterBalanceIsExposed) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.MaxHarvesterBalance = Amount(446633);
		AccountStateCache originalCache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(Amount(446633), ReadOnlyAccountStateCache(*originalCache.createView()).maxHarvesterBalance());
		EXPECT_EQ(Amount(446633), ReadOnlyAccountStateCache(*originalCache.createDelta()).maxHarvesterBalance());
	}

	TEST(TEST_CLASS, HarvestingMosaicIdIsExposed) {
		// Arrange:
		auto options = Default_Cache_Options;
		options.HarvestingMosaicId = MosaicId(11229988);
		AccountStateCache originalCache(CacheConfiguration(), options);

		// Act + Assert:
		EXPECT_EQ(MosaicId(11229988), ReadOnlyAccountStateCache(*originalCache.createView()).harvestingMosaicId());
		EXPECT_EQ(MosaicId(11229988), ReadOnlyAccountStateCache(*originalCache.createDelta()).harvestingMosaicId());
	}

	// endregion

	namespace {
		struct AccountStateCacheByAddressTraits {
			static Address CreateKey(uint8_t tag) {
				return { { static_cast<uint8_t>(tag * tag) } };
			}

			static Address GetKey(const state::AccountState& accountState) {
				return accountState.Address;
			}
		};

		struct AccountStateCacheByKeyTraits {
			static Key CreateKey(uint8_t tag) {
				return { { static_cast<uint8_t>(tag * tag) } };
			}

			static Key GetKey(const state::AccountState& accountState) {
				return accountState.PublicKey;
			}
		};
	}

#define ACCOUNT_KEY_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ByAddress) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountStateCacheByAddressTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ByKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountStateCacheByKeyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyContainsCommittedElements) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateKey(1)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateKey(2)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateKey(3)));
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaContainsBothCommittedAndUncommittedElements) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto cacheDelta = cache.createDelta();
		cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed
		cache.commit();
		cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateKey(1)));
		EXPECT_TRUE(readOnlyCache.contains(TTraits::CreateKey(2)));
		EXPECT_FALSE(readOnlyCache.contains(TTraits::CreateKey(3)));
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyCanAccessCommittedElementsViaGet) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed;
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_EQ(TTraits::CreateKey(1), TTraits::GetKey(readOnlyCache.find(TTraits::CreateKey(1)).get()));
		EXPECT_THROW(readOnlyCache.find(TTraits::CreateKey(2)).get(), catapult_invalid_argument);
		EXPECT_THROW(readOnlyCache.find(TTraits::CreateKey(3)).get(), catapult_invalid_argument);
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaCanAccessBothCommittedAndUncommittedElementsViaGet) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto cacheDelta = cache.createDelta();
		cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed
		cache.commit();
		cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_EQ(TTraits::CreateKey(1), TTraits::GetKey(readOnlyCache.find(TTraits::CreateKey(1)).get()));
		EXPECT_EQ(TTraits::CreateKey(2), TTraits::GetKey(readOnlyCache.find(TTraits::CreateKey(2)).get()));
		EXPECT_THROW(readOnlyCache.find(TTraits::CreateKey(3)).get(), catapult_invalid_argument);
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyViewOnlyCanAccessCommittedElementsViaTryGet) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		{
			auto cacheDelta = cache.createDelta();
			cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed;
			cache.commit();
			cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted
		}

		// Act:
		auto cacheView = cache.createView();
		ReadOnlyAccountStateCache readOnlyCache(*cacheView);

		// Assert:
		EXPECT_EQ(1u, readOnlyCache.size());
		EXPECT_EQ(TTraits::CreateKey(1), TTraits::GetKey(*readOnlyCache.find(TTraits::CreateKey(1)).tryGet()));
		EXPECT_FALSE(!!readOnlyCache.find(TTraits::CreateKey(2)).tryGet());
		EXPECT_FALSE(!!readOnlyCache.find(TTraits::CreateKey(3)).tryGet());
	}

	ACCOUNT_KEY_BASED_TEST(ReadOnlyDeltaCanAccessBothCommittedAndUncommittedElementsViaTryGet) {
		// Arrange:
		AccountStateCache cache(CacheConfiguration(), Default_Cache_Options);
		auto cacheDelta = cache.createDelta();
		cacheDelta->addAccount(TTraits::CreateKey(1), Height(123)); // committed
		cache.commit();
		cacheDelta->addAccount(TTraits::CreateKey(2), Height(123)); // uncommitted

		// Act:
		ReadOnlyAccountStateCache readOnlyCache(*cacheDelta);

		// Assert:
		EXPECT_EQ(2u, readOnlyCache.size());
		EXPECT_EQ(TTraits::CreateKey(1), TTraits::GetKey(*readOnlyCache.find(TTraits::CreateKey(1)).tryGet()));
		EXPECT_EQ(TTraits::CreateKey(2), TTraits::GetKey(*readOnlyCache.find(TTraits::CreateKey(2)).tryGet()));
		EXPECT_FALSE(!!readOnlyCache.find(TTraits::CreateKey(3)).tryGet());
	}
}}
