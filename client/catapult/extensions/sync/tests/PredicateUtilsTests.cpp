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

#include "sync/src/PredicateUtils.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS PredicateUtilsTests

	// region ToUnknownTransactionPredicate

	TEST(TEST_CLASS, ToUnknownTransactionPredicateDelegatesToKnownHashPredicateForTransactionType) {
		// Arrange:
		auto entityType = model::BasicEntityType::Transaction;
		auto knownHash = test::GenerateRandomData<Hash256_Size>();
		auto otherHash = test::GenerateRandomData<Hash256_Size>();
		auto predicate = ToUnknownTransactionPredicate([&knownHash](auto timestamp, const auto& hash) {
			return Timestamp(123) == timestamp && knownHash == hash;
		});

		// Act + Assert:
		EXPECT_FALSE(predicate(entityType, Timestamp(123), knownHash)); // match
		EXPECT_TRUE(predicate(entityType, Timestamp(245), knownHash)); // diff timestamp
		EXPECT_TRUE(predicate(entityType, Timestamp(123), otherHash)); // diff hash
		EXPECT_TRUE(predicate(entityType, Timestamp(245), otherHash)); // diff timestamp + hash
	}

	TEST(TEST_CLASS, ToUnknownTransactionPredicateAlwaysReturnsFalseForNonTransactionType) {
		// Arrange:
		auto entityType = model::BasicEntityType::Block;
		auto knownHash = test::GenerateRandomData<Hash256_Size>();
		auto otherHash = test::GenerateRandomData<Hash256_Size>();
		auto predicate = ToUnknownTransactionPredicate([&knownHash](auto timestamp, const auto& hash) {
			return Timestamp(123) == timestamp && knownHash == hash;
		});

		// Act + Assert:
		EXPECT_FALSE(predicate(entityType, Timestamp(123), knownHash)); // match
		EXPECT_FALSE(predicate(entityType, Timestamp(245), knownHash)); // diff timestamp
		EXPECT_FALSE(predicate(entityType, Timestamp(123), otherHash)); // diff hash
		EXPECT_FALSE(predicate(entityType, Timestamp(245), otherHash)); // diff timestamp + hash
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
			bool ShouldEnableTransactionSpamThrottling;
			uint32_t MaxCacheSize;
			uint32_t MaxBlockSize;
		};

		config::LocalNodeConfiguration CreateLocalNodeConfigurationFromSettings(const ThrottleTestSettings& settings) {
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			blockChainConfig.TotalChainBalance = utils::XemUnit(utils::XemAmount(1'000'000'000));
			blockChainConfig.MaxTransactionsPerBlock = settings.MaxBlockSize;
			blockChainConfig.ImportanceGrouping = 1;

			auto nodeConfig = config::NodeConfiguration::Uninitialized();
			nodeConfig.ShouldEnableTransactionSpamThrottling = settings.ShouldEnableTransactionSpamThrottling;
			nodeConfig.TransactionSpamThrottlingMaxBoostFee = Amount(10'000'000);
			nodeConfig.UnconfirmedTransactionsCacheMaxSize = settings.MaxCacheSize;

			return config::LocalNodeConfiguration(
					std::move(blockChainConfig),
					std::move(nodeConfig),
					config::LoggingConfiguration::Uninitialized(),
					config::UserConfiguration::Uninitialized());
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
			auto config = CreateLocalNodeConfigurationFromSettings(settings);

			// - create a read only catapult cache (importance grouping needs to be nonzero)
			auto catapultCache = test::CreateEmptyCatapultCache(config.BlockChain);
			auto catapultCacheView = catapultCache.createView();
			auto readOnlyCatapultCache = catapultCacheView.toReadOnly();

			// - create a ut cache with some transactions
			cache::MemoryUtCache utCache(cache::MemoryCacheOptions(1024, 1024));
			auto utCacheModifier = utCache.modifier();
			for (const auto& transactionInfo : test::CreateTransactionInfos(cacheSize))
				utCacheModifier.add(transactionInfo);

			auto throttleContext = CreateThrottleContext(readOnlyCatapultCache, utCacheModifier);

			auto pTransaction = utils::UniqueToShared(test::GenerateRandomTransaction());
			pTransaction->Type = transactionType;
			pTransaction->Fee = Amount(2'000'000);

			// Sanity:
			EXPECT_EQ(cacheSize, utCacheModifier.size());

			// Act:
			auto isThrottled = CreateUtUpdaterThrottle(config)(model::TransactionInfo(pTransaction), throttleContext);

			// Assert:
			EXPECT_EQ(expectedIsThrottled, isThrottled)
					<< "cacheSize = " << cacheSize
					<< ", maxCacheSize = " << settings.MaxCacheSize
					<< ", maxBlockSize = " << settings.MaxBlockSize;
		}

		constexpr ThrottleTestSettings No_Spam_Test_Settings = { false, 100, 50 };
		constexpr ThrottleTestSettings Spam_Test_Settings = { true, 100, 50 };
	}

	// endregion

	// region CreateUtUpdaterThrottle (no spam filtering)

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsFalseWhenCacheSizeIsLessThanMaxCacheSize_NoSpamFiltering) {
		// Assert:
		AssertUtUpdaterThrottleResult(false, No_Spam_Test_Settings, 0);
		AssertUtUpdaterThrottleResult(false, No_Spam_Test_Settings, 49);
		AssertUtUpdaterThrottleResult(false, No_Spam_Test_Settings, 51);
		AssertUtUpdaterThrottleResult(false, No_Spam_Test_Settings, 99);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsTrueWhenCacheSizeIsEqualToMaxCacheSize_NoSpamFiltering) {
		// Assert:
		AssertUtUpdaterThrottleResult(true, No_Spam_Test_Settings, 100);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsTrueWhenCacheSizeIsGreaterThanMaxCacheSize_NoSpamFiltering) {
		// Assert:
		AssertUtUpdaterThrottleResult(true, No_Spam_Test_Settings, 101);
		AssertUtUpdaterThrottleResult(true, No_Spam_Test_Settings, 200);
	}

	// endregion

	// region CreateUtUpdaterThrottle (spam filtering)

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsFalseWhenCacheSizeIsLessThanMaxBlockSize_SpamFiltering) {
		// Assert:
		AssertUtUpdaterThrottleResult(false, Spam_Test_Settings, 0);
		AssertUtUpdaterThrottleResult(false, Spam_Test_Settings, 49);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleIsInfluencedByCacheFillLevel_SpamFiltering) {
		// Act: even though transaction has high fee, it is throttled when cache is nearly full
		AssertUtUpdaterThrottleResult(false, Spam_Test_Settings, 51);
		AssertUtUpdaterThrottleResult(true, Spam_Test_Settings, 99);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleIsInfluencedByTransactionType_SpamFiltering) {
		// Act: cache throttling is only bypassed for aggregate bonded transactions
		AssertUtUpdaterThrottleResult(true, Spam_Test_Settings, 99, Normal_Transaction_Type);
		AssertUtUpdaterThrottleResult(true, Spam_Test_Settings, 99, Aggregate_Transaction_Type);
		AssertUtUpdaterThrottleResult(false, Spam_Test_Settings, 99, Aggregate_Bonded_Transaction_Type);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsTrueWhenCacheSizeIsEqualToMaxCacheSize_SpamFiltering) {
		// Assert:
		AssertUtUpdaterThrottleResult(true, Spam_Test_Settings, 100);
	}

	TEST(TEST_CLASS, UtUpdaterThrottleReturnsTrueWhenCacheSizeIsGreaterThanMaxCacheSize_SpamFiltering) {
		// Assert:
		AssertUtUpdaterThrottleResult(true, Spam_Test_Settings, 101);
		AssertUtUpdaterThrottleResult(true, Spam_Test_Settings, 200);
	}

	// endregion
}}
