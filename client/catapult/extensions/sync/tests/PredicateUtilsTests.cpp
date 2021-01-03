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

#include "sync/src/PredicateUtils.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/other/MutableCatapultConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS PredicateUtilsTests

	// region ToRequiresValidationPredicate

	TEST(TEST_CLASS, ToRequiresValidationPredicateDelegatesToKnownHashPredicateForTransactionType) {
		// Arrange:
		auto entityType = model::BasicEntityType::Transaction;
		auto knownHash = test::GenerateRandomByteArray<Hash256>();
		auto otherHash = test::GenerateRandomByteArray<Hash256>();
		auto predicate = ToRequiresValidationPredicate([&knownHash](auto timestamp, const auto& hash) {
			return Timestamp(123) == timestamp && knownHash == hash;
		});

		// Act + Assert:
		EXPECT_FALSE(predicate(entityType, Timestamp(123), knownHash)); // match
		EXPECT_TRUE(predicate(entityType, Timestamp(245), knownHash)); // diff timestamp
		EXPECT_TRUE(predicate(entityType, Timestamp(123), otherHash)); // diff hash
		EXPECT_TRUE(predicate(entityType, Timestamp(245), otherHash)); // diff timestamp + hash
	}

	namespace {
		void AssertAlwaysRequiresValidation(model::BasicEntityType entityType) {
			// Arrange:
			auto knownHash = test::GenerateRandomByteArray<Hash256>();
			auto otherHash = test::GenerateRandomByteArray<Hash256>();
			auto predicate = ToRequiresValidationPredicate([&knownHash](auto timestamp, const auto& hash) {
				return Timestamp(123) == timestamp && knownHash == hash;
			});

			// Act + Assert:
			EXPECT_TRUE(predicate(entityType, Timestamp(123), knownHash)); // match
			EXPECT_TRUE(predicate(entityType, Timestamp(245), knownHash)); // diff timestamp
			EXPECT_TRUE(predicate(entityType, Timestamp(123), otherHash)); // diff hash
			EXPECT_TRUE(predicate(entityType, Timestamp(245), otherHash)); // diff timestamp + hash
		}
	}

	TEST(TEST_CLASS, ToRequiresValidationPredicateAlwaysReturnsTrueForBlockType) {
		AssertAlwaysRequiresValidation(model::BasicEntityType::Block);
	}

	TEST(TEST_CLASS, ToRequiresValidationPredicateAlwaysReturnsTrueForOtherType) {
		AssertAlwaysRequiresValidation(static_cast<model::BasicEntityType>(88));
	}

	// endregion

	// region CreateUtUpdaterThrottle - setup

	namespace {
		constexpr auto MakeTransactionType(model::FacilityCode facilityCode, uint8_t code) {
			return model::MakeEntityType(model::BasicEntityType::Transaction, facilityCode, code);
		}

		constexpr auto Normal_Transaction_Type = MakeTransactionType(model::FacilityCode::Transfer, 2);
		constexpr auto Aggregate_Transaction_Type = MakeTransactionType(model::FacilityCode::Aggregate, 1);
		constexpr auto Aggregate_Bonded_Transaction_Type = MakeTransactionType(model::FacilityCode::Aggregate, 2);

		struct ThrottleTestSettings {
			bool EnableTransactionSpamThrottling;
			uint64_t MaxCacheSize;
			uint32_t MaxBlockSize;
		};

		config::CatapultConfiguration CreateCatapultConfigurationFromSettings(const ThrottleTestSettings& settings) {
			test::MutableCatapultConfiguration config;
			config.BlockChain.TotalChainImportance = Importance(1'000'000'000);
			config.BlockChain.MaxTransactionsPerBlock = settings.MaxBlockSize;
			config.BlockChain.ImportanceGrouping = 1;

			config.Node.EnableTransactionSpamThrottling = settings.EnableTransactionSpamThrottling;
			config.Node.TransactionSpamThrottlingMaxBoostFee = Amount(10'000'000);
			config.Node.UnconfirmedTransactionsCacheMaxSize = utils::FileSize::FromBytes(settings.MaxCacheSize);
			return config.ToConst();
		}

		chain::UtUpdater::ThrottleContext CreateThrottleContext(
				const cache::ReadOnlyCatapultCache& readOnlyCatapultCache,
				const cache::UtCacheModifierProxy& utCacheModifier) {
			return { chain::UtUpdater::TransactionSource::New, Height(), readOnlyCatapultCache, utCacheModifier };
		}

		void AssertUtUpdaterThrottleResult(
				bool expectedIsThrottled,
				const ThrottleTestSettings& settings,
				uint32_t cacheSize,
				model::EntityType transactionType = Normal_Transaction_Type) {
			// Arrange:
			auto config = CreateCatapultConfigurationFromSettings(settings);

			// - create a read only catapult cache (importance grouping needs to be nonzero)
			auto catapultCache = test::CreateEmptyCatapultCache(config.BlockChain);
			auto catapultCacheView = catapultCache.createView();
			auto readOnlyCatapultCache = catapultCacheView.toReadOnly();

			// - create a ut cache with some transactions
			cache::MemoryUtCache utCache(cache::MemoryCacheOptions(utils::FileSize(), utils::FileSize::FromBytes(settings.MaxCacheSize)));
			auto utCacheModifier = utCache.modifier();
			for (const auto& transactionInfo : test::CreateTransactionInfos(cacheSize))
				utCacheModifier.add(transactionInfo);

			auto throttleContext = CreateThrottleContext(readOnlyCatapultCache, utCacheModifier);

			auto pTransaction = utils::UniqueToShared(test::GenerateRandomTransaction());
			pTransaction->Type = transactionType;
			pTransaction->MaxFee = Amount(2'000'000);

			// Act:
			auto isThrottled = CreateUtUpdaterThrottle(config)(model::TransactionInfo(pTransaction), throttleContext);

			// Assert:
			EXPECT_EQ(expectedIsThrottled, isThrottled)
					<< "cacheSize = " << cacheSize
					<< ", maxCacheSize = " << settings.MaxCacheSize
					<< ", maxBlockSize = " << settings.MaxBlockSize;
		}

		ThrottleTestSettings SpamThrottleDisabledTestSettings() {
			return { false, 100 * test::GetDefaultRandomTransactionSize(), 50 };
		}

		ThrottleTestSettings SpamThrottleEnabledTestSettings() {
			return { true, 100 * test::GetDefaultRandomTransactionSize(), 50 };
		}
	}

	// endregion

	// region CreateUtUpdaterThrottle (no spam filtering)

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsFalseWhenCacheSizeIsLessThanMaxCacheSize_NoSpamFiltering) {
		AssertUtUpdaterThrottleResult(false, SpamThrottleDisabledTestSettings(), 0);
		AssertUtUpdaterThrottleResult(false, SpamThrottleDisabledTestSettings(), 49);
		AssertUtUpdaterThrottleResult(false, SpamThrottleDisabledTestSettings(), 51);
		AssertUtUpdaterThrottleResult(false, SpamThrottleDisabledTestSettings(), 99);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsTrueWhenCacheSizeIsEqualToMaxCacheSize_NoSpamFiltering) {
		AssertUtUpdaterThrottleResult(true, SpamThrottleDisabledTestSettings(), 100);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsTrueWhenCacheSizeIsGreaterThanMaxCacheSize_NoSpamFiltering) {
		AssertUtUpdaterThrottleResult(true, SpamThrottleDisabledTestSettings(), 101);
		AssertUtUpdaterThrottleResult(true, SpamThrottleDisabledTestSettings(), 200);
	}

	// endregion

	// region CreateUtUpdaterThrottle (spam filtering)

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsFalseWhenCacheSizeIsLessThanMaxBlockSize_SpamFiltering) {
		AssertUtUpdaterThrottleResult(false, SpamThrottleEnabledTestSettings(), 0);
		AssertUtUpdaterThrottleResult(false, SpamThrottleEnabledTestSettings(), 49);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleIsInfluencedByCacheFillLevel_SpamFiltering) {
		// Act: even though transaction has high fee, it is throttled when cache is nearly full
		AssertUtUpdaterThrottleResult(false, SpamThrottleEnabledTestSettings(), 51);
		AssertUtUpdaterThrottleResult(true, SpamThrottleEnabledTestSettings(), 99);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleIsInfluencedByTransactionType_SpamFiltering) {
		// Act: cache throttling is only bypassed for aggregate bonded transactions
		AssertUtUpdaterThrottleResult(true, SpamThrottleEnabledTestSettings(), 99, Normal_Transaction_Type);
		AssertUtUpdaterThrottleResult(true, SpamThrottleEnabledTestSettings(), 99, Aggregate_Transaction_Type);
		AssertUtUpdaterThrottleResult(false, SpamThrottleEnabledTestSettings(), 99, Aggregate_Bonded_Transaction_Type);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsTrueWhenCacheSizeIsEqualToMaxCacheSize_SpamFiltering) {
		AssertUtUpdaterThrottleResult(true, SpamThrottleEnabledTestSettings(), 100);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsTrueWhenCacheSizeIsGreaterThanMaxCacheSize_SpamFiltering) {
		AssertUtUpdaterThrottleResult(true, SpamThrottleEnabledTestSettings(), 101);
		AssertUtUpdaterThrottleResult(true, SpamThrottleEnabledTestSettings(), 200);
	}

	// endregion
}}
