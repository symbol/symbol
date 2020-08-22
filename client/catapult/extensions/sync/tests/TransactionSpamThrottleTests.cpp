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

#include "sync/src/TransactionSpamThrottle.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/HeightGrouping.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS TransactionSpamThrottleTests

	namespace {
		using TransactionSource = chain::UtUpdater::TransactionSource;

		cache::CatapultCache CreateCatapultCacheWithImportanceGrouping(uint64_t importanceGrouping) {
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			blockChainConfig.ImportanceGrouping = importanceGrouping;
			return test::CreateEmptyCatapultCache(blockChainConfig);
		}

		Key AddAccount(cache::AccountStateCacheDelta& delta, Importance importance) {
			auto publicKey = test::GenerateRandomByteArray<Key>();
			delta.addAccount(publicKey, Height(1));
			auto& accountState = delta.find(publicKey).get();
			accountState.ImportanceSnapshots.set(importance, model::ImportanceHeight(1));
			return publicKey;
		}

		std::vector<Key> SeedAccountStateCache(cache::AccountStateCacheDelta& delta, size_t count, Importance importance) {
			std::vector<Key> publicKeys;
			for (auto i = 0u; i < count; ++i)
				publicKeys.push_back(AddAccount(delta, importance));

			return publicKeys;
		}

		model::TransactionInfo CreateTransactionInfo(const Key& signer, Amount fee = Amount()) {
			auto pTransaction = test::GenerateRandomTransaction(signer);
			pTransaction->MaxFee = fee;
			auto transactionInfo = model::TransactionInfo(std::move(pTransaction));
			test::FillWithRandomData(transactionInfo.EntityHash);
			return transactionInfo;
		}

		class TestContext {
		public:
			TestContext(cache::CatapultCache&& catapultCache, Height height)
					: m_catapultCache(std::move(catapultCache))
					, m_catapultCacheView(m_catapultCache.createView())
					, m_readOnlyCatapultCache(m_catapultCacheView.toReadOnly())
					, m_height(height)
					, m_transactionsCache(cache::MemoryCacheOptions(1024, 100'000))
					, m_transactionsCacheModifier(m_transactionsCache.modifier())
			{}

		public:
			cache::UtCacheModifierProxy& transactionsCacheModifier() {
				return m_transactionsCacheModifier;
			}

			chain::UtUpdater::ThrottleContext throttleContext(TransactionSource source = TransactionSource::New) const {
				return { source, m_height, m_readOnlyCatapultCache, m_transactionsCacheModifier };
			}

			void addTransactions(const std::vector<Key>& publicKeys) {
				for (const auto& publicKey : publicKeys)
					m_transactionsCacheModifier.add(CreateTransactionInfo(publicKey));
			}

		private:
			cache::CatapultCache m_catapultCache;
			cache::CatapultCacheView m_catapultCacheView;
			cache::ReadOnlyCatapultCache m_readOnlyCatapultCache;
			Height m_height;
			cache::MemoryUtCacheProxy m_transactionsCache;
			cache::UtCacheModifierProxy m_transactionsCacheModifier;
		};

		// region ThrottleTestSettings + factory functions

		enum class TransactionBondPolicy { Unbonded, Bonded };

		struct ThrottleTestSettings {
			SpamThrottleConfiguration ThrottleConfig{ Amount(10'000'000), Importance(1'000'000), 1'200, 120 };
			uint32_t CacheSize = 120;
			Importance DefaultImportance = Importance(1'000);
			Importance SignerImportance = Importance();
			Amount Fee = Amount();
			TransactionSource Source = TransactionSource::New;
			TransactionBondPolicy BondPolicy = TransactionBondPolicy::Unbonded;
		};

		ThrottleTestSettings CreateSettingsWithCacheSize(uint32_t cacheSize) {
			ThrottleTestSettings settings;
			settings.CacheSize = cacheSize;
			return settings;
		}

		ThrottleTestSettings CreateSettingsWithSignerImportance(Importance signerImportance) {
			ThrottleTestSettings settings;
			settings.SignerImportance = signerImportance;
			return settings;
		}

		ThrottleTestSettings CreateSettingsWithFee(Amount fee) {
			ThrottleTestSettings settings;
			settings.Fee = fee;
			return settings;
		}

		ThrottleTestSettings CreateSettingsWithFee(Amount fee, Amount maxBoostFee) {
			ThrottleTestSettings settings;
			settings.Fee = fee;
			settings.ThrottleConfig.MaxBoostFee = maxBoostFee;
			return settings;
		}

		ThrottleTestSettings CreateSettings(uint32_t cacheSize, Importance signerImportance, Amount fee) {
			ThrottleTestSettings settings;
			settings.CacheSize = cacheSize;
			settings.SignerImportance = signerImportance;
			settings.Fee = fee;
			return settings;
		}

		ThrottleTestSettings CreateSettings(uint32_t cacheMaxSize, uint32_t cacheSize, TransactionSource source) {
			ThrottleTestSettings settings;
			settings.ThrottleConfig.MaxCacheSize = cacheMaxSize;
			settings.ThrottleConfig.MaxBlockSize = 10;
			settings.CacheSize = cacheSize;
			settings.DefaultImportance = Importance();
			settings.Source = source;
			return settings;
		}

		ThrottleTestSettings CreateSettings(uint32_t cacheMaxSize, uint32_t cacheSize, TransactionBondPolicy bondPolicy) {
			auto settings = CreateSettings(cacheMaxSize, cacheSize, TransactionSource::New);
			settings.BondPolicy = bondPolicy;
			return settings;
		}

		// endregion

		void AssertThrottling(const ThrottleTestSettings& settings, bool expectedResult) {
			// Arrange: prepare account state cache
			auto catapultCache = CreateCatapultCacheWithImportanceGrouping(100);
			std::vector<Key> publicKeys;
			Key signerPublicKey;
			{
				auto delta = catapultCache.createDelta();
				auto& accountStateCacheDelta = delta.sub<cache::AccountStateCache>();
				publicKeys = SeedAccountStateCache(accountStateCacheDelta, settings.CacheSize, settings.DefaultImportance);
				signerPublicKey = AddAccount(accountStateCacheDelta, settings.SignerImportance);
				catapultCache.commit(Height(1));
			}

			TestContext context(std::move(catapultCache), Height(1));
			context.addTransactions(publicKeys);

			// Sanity:
			EXPECT_EQ(settings.CacheSize, context.transactionsCacheModifier().size());

			std::vector<const model::Transaction*> transactions;
			auto isBonded = [&transactions, bondPolicy = settings.BondPolicy](const auto& transaction) {
				transactions.push_back(&transaction);
				return TransactionBondPolicy::Bonded == bondPolicy;
			};
			auto throttleContext = context.throttleContext(settings.Source);
			auto transactionInfo = CreateTransactionInfo(signerPublicKey, settings.Fee);
			auto filter = CreateTransactionSpamThrottle(settings.ThrottleConfig, isBonded);

			// Act:
			auto result = filter(transactionInfo, throttleContext);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "for importance " << settings.SignerImportance;
			auto expectedTransactions =
					settings.CacheSize >= settings.ThrottleConfig.MaxBlockSize && settings.CacheSize < settings.ThrottleConfig.MaxCacheSize
					? std::vector<const model::Transaction*>{ transactionInfo.pEntity.get() }
					: std::vector<const model::Transaction*>();
			EXPECT_EQ(expectedTransactions, transactions) << "for importance " << settings.SignerImportance;
		}
	}

	// region comparison of cache size to block size

	TEST(TEST_CLASS, TransactionIsNotFilteredWhenCacheSizeIsSmallerThanBlockSize) {
		// Act: max block size is 120
		AssertThrottling(CreateSettingsWithCacheSize(0), false);
		AssertThrottling(CreateSettingsWithCacheSize(1), false);
		AssertThrottling(CreateSettingsWithCacheSize(10), false);
		AssertThrottling(CreateSettingsWithCacheSize(119), false);
	}

	TEST(TEST_CLASS, TransactionIsFilteredWhenCacheSizeIsEqualToBlockSizeAndTransactionDoesNotMeetRequirements) {
		// Act: max block size is 120
		AssertThrottling(CreateSettingsWithCacheSize(120), true);
	}

	TEST(TEST_CLASS, TransactionIsFilteredWhenCacheSizeIsGreaterThanBlockSizeAndTransactionDoesNotMeetRequirements) {
		// Act: max block size is 120
		AssertThrottling(CreateSettingsWithCacheSize(121), true);
		AssertThrottling(CreateSettingsWithCacheSize(200), true);
		AssertThrottling(CreateSettingsWithCacheSize(500), true);
	}

	// endregion

	// region importance

	TEST(TEST_CLASS, ImportanceIncreasesEffectiveImportance) {
		// Act: signer has no importance
		AssertThrottling(CreateSettingsWithSignerImportance(Importance()), true);

		// Act: signer has 0.1% importance of total importance, transaction is accepted
		AssertThrottling(CreateSettingsWithSignerImportance(Importance(1'000)), false);
	}

	// endregion

	// region fee

	TEST(TEST_CLASS, TransactionFeeIncreasesEffectiveImportance) {
		// Act: no fee, transaction is rejected
		AssertThrottling(CreateSettingsWithFee(Amount()), true);

		// Act: high fee boosts the effective importance to 1% of total importance:
		//      - total importance = 1'000'000, fee = 10 currency, so attemptedImportanceBoost = 10'000
		AssertThrottling(CreateSettingsWithFee(Amount(10'000'000)), false);
	}

	TEST(TEST_CLASS, MaxBoostFeeAffectsHowTransactionFeeIncreasesEffectiveImportance) {
		// Act: the max boost fee is 10 currency, a low fee is not enough to get the transactions accepted
		//      - low fee boosts the effective importance by only 0.001% of total importance:
		//      - total importance = 1'000'000, fee = 0.01 currency, max boost fee = 10 currency, so attemptedImportanceBoost = 10
		AssertThrottling(CreateSettingsWithFee(Amount(10'000), Amount(10'000'000)), true);

		// Act: the max boost fee is 0.01 currency, even a low fee is enough to get the transactions accepted
		//      - even a low fee boosts the effective importance to 1% of total importance:
		//      - total importance = 1'000'000, fee = 0.01 currency, max boost fee = 0.01 currency, so attemptedImportanceBoost = 10'000
		AssertThrottling(CreateSettingsWithFee(Amount(10'000), Amount(10'000)), false);
	}

	// endregion

	// region throttling - single account / multiple accounts

	namespace {
		enum class AccountPolicy { Single, Multiple };

		void AssertThrottling(const ThrottleTestSettings& settings, AccountPolicy accountPolicy, uint32_t expectedCacheSize) {
			// Arrange:
			auto config = settings.ThrottleConfig;
			auto catapultCache = CreateCatapultCacheWithImportanceGrouping(100);
			std::vector<Key> publicKeys;
			{
				auto delta = catapultCache.createDelta();
				auto& accountStateCacheDelta = delta.sub<cache::AccountStateCache>();
				publicKeys = SeedAccountStateCache(accountStateCacheDelta, config.MaxCacheSize, settings.SignerImportance);
				catapultCache.commit(Height(1));
			}

			TestContext context(std::move(catapultCache), Height(1));

			// Sanity:
			EXPECT_EQ(0u, context.transactionsCacheModifier().size());

			auto throttleContext = context.throttleContext();

			bool isFiltered = false;
			auto index = 0u;
			while (!isFiltered && config.MaxCacheSize > context.transactionsCacheModifier().size()) {
				auto transactionInfo = CreateTransactionInfo(publicKeys[index], settings.Fee);
				auto filter = CreateTransactionSpamThrottle(config, [](const auto&) { return false; });

				// Act:
				isFiltered = filter(transactionInfo, throttleContext);

				// Assert:
				auto cacheSize = context.transactionsCacheModifier().size();
				if (!isFiltered && !context.transactionsCacheModifier().add(transactionInfo))
					CATAPULT_THROW_RUNTIME_ERROR_1("transaction could not be added to cache at size", cacheSize);

				if (AccountPolicy::Multiple == accountPolicy)
					++index;
			}

			// Assert:
			EXPECT_EQ(expectedCacheSize, context.transactionsCacheModifier().size()) << "for importance " << settings.SignerImportance;
		}
	}

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_SingleAccount) {
		// Arrange:
		// - single account fills the cache
		// - rounded solutions for equation: importance * e^(-3 * y / 1200) * 100 * (1200 - y) = y
		// - for a relative importance <= 0.001, only max transactions per block (120) are allowed when cache is initially empty
		std::vector<uint64_t> rawImportances{ 1'000'000, 100'000, 10'000, 1000, 100, 10 };
		std::vector<uint32_t> expectedCacheSizes{ 1054, 736, 352, 120, 120, 120 };
		for (auto i = 0u; i < rawImportances.size(); ++i) {
			auto settings = CreateSettings(0, Importance(rawImportances[i]), Amount());
			AssertThrottling(settings, AccountPolicy::Single, expectedCacheSizes[i]);
		}
	}

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_MultipleAccounts) {
		// Arrange:
		// - different accounts fill the cache
		// - rounded solutions for equation: importance * e^(-3 * y / 1200) * 100 * (1200 - y) = 1
		std::vector<uint64_t> rawImportances{ 1'000'000, 100'000, 10'000, 1000, 100, 10 };
		std::vector<uint32_t> expectedCacheSizes{ 1200, 1199, 1181, 1059, 669, 120 };
		for (auto i = 0u; i < rawImportances.size(); ++i) {
			auto settings = CreateSettings(0, Importance(rawImportances[i]), Amount());
			AssertThrottling(settings, AccountPolicy::Multiple, expectedCacheSizes[i]);
		}
	}

	// endregion

	// region throttling - different fill levels

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_FullCache) {
		// Act + Assert: max importance to try to force acceptance
		AssertThrottling(CreateSettings(1199, Importance(1'000'000), Amount()), false); // almost full
		AssertThrottling(CreateSettings(1200, Importance(1'000'000), Amount()), true); // full
	}

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_HighFillLevel) {
		AssertThrottling(CreateSettings(1000, Importance(), Amount()), true); // no importance and fee
		AssertThrottling(CreateSettings(1000, Importance(), Amount(100'000)), true); // no importance, medium fee
		AssertThrottling(CreateSettings(1000, Importance(), Amount(1000'000)), false); // no importance, high fee

		AssertThrottling(CreateSettings(1000, Importance(100), Amount()), true); // medium importance and no fee
		AssertThrottling(CreateSettings(1000, Importance(100), Amount(100'000)), true); // medium importance and fee

		AssertThrottling(CreateSettings(1000, Importance(10'000), Amount()), false); // high importance and no fee
		AssertThrottling(CreateSettings(1000, Importance(10'000), Amount(1'000'000)), false); // high importance and fee
	}

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_MediumFillLevel) {
		AssertThrottling(CreateSettings(250, Importance(), Amount()), true); // no importance and fee
		AssertThrottling(CreateSettings(250, Importance(), Amount(100'000)), false); // no importance, medium fee
		AssertThrottling(CreateSettings(250, Importance(), Amount(1000'000)), false); // no importance, high fee

		AssertThrottling(CreateSettings(250, Importance(100), Amount()), false); // medium importance and no fee
		AssertThrottling(CreateSettings(250, Importance(100), Amount(100'000)), false); // medium importance and fee

		AssertThrottling(CreateSettings(250, Importance(10'000), Amount()), false); // high importance and no fee
		AssertThrottling(CreateSettings(250, Importance(10'000), Amount(1'000'000)), false); // high importance and fee
	}

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_LowFillLevel) {
		AssertThrottling(CreateSettings(100, Importance(), Amount()), false); // no importance and fee
		AssertThrottling(CreateSettings(100, Importance(), Amount(100'000)), false); // no importance, medium fee
		AssertThrottling(CreateSettings(100, Importance(), Amount(1000'000)), false); // no importance, high fee

		AssertThrottling(CreateSettings(100, Importance(100), Amount()), false); // medium importance and no fee
		AssertThrottling(CreateSettings(100, Importance(100), Amount(100'000)), false); // medium importance and fee

		AssertThrottling(CreateSettings(100, Importance(10'000), Amount()), false); // high importance and no fee
		AssertThrottling(CreateSettings(100, Importance(10'000), Amount(1'000'000)), false); // high importance and fee
	}

	// endregion

	// region throttling - transaction source

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_TransactionSourceNew) {
		// Act + Assert: block size is 10, no importance and fee
		AssertThrottling(CreateSettings(100, 9, TransactionSource::New), false); // cache size < block max size
		AssertThrottling(CreateSettings(100, 10, TransactionSource::New), true); // cache size == block max size
		AssertThrottling(CreateSettings(100, 50, TransactionSource::New), true); // cache size > block max size
	}

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_TransactionSourceExisting) {
		// Act + Assert: block size is 10, no importance and fee
		AssertThrottling(CreateSettings(100, 9, TransactionSource::Existing), false); // cache size < block max size
		AssertThrottling(CreateSettings(100, 10, TransactionSource::Existing), true); // cache size == block max size
		AssertThrottling(CreateSettings(100, 50, TransactionSource::Existing), true); // cache size > block max size
	}

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_TransactionSourceReverted) {
		// Act + Assert: block size is 10, no importance and fee
		AssertThrottling(CreateSettings(100, 50, TransactionSource::Reverted), false); // cache is half full
		AssertThrottling(CreateSettings(100, 99, TransactionSource::Reverted), false); // cache is almost full
		AssertThrottling(CreateSettings(100, 100, TransactionSource::Reverted), true); // cache is full
	}

	// endregion

	// region throttling - bonded transactions

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_BondedTransactions) {
		// Act + Assert: block size is 10, no importance and fee
		AssertThrottling(CreateSettings(100, 50, TransactionBondPolicy::Bonded), false); // cache is half full
		AssertThrottling(CreateSettings(100, 99, TransactionBondPolicy::Bonded), false); // cache is almost full
		AssertThrottling(CreateSettings(100, 100, TransactionBondPolicy::Bonded), true); // cache is full
	}

	TEST(TEST_CLASS, ThrottlingBehavesAsExpected_UnbondedTransactions) {
		// Act + Assert: block size is 10, no importance and fee
		AssertThrottling(CreateSettings(100, 9, TransactionBondPolicy::Unbonded), false); // cache size < block max size
		AssertThrottling(CreateSettings(100, 10, TransactionBondPolicy::Unbonded), true); // cache size == block max size
		AssertThrottling(CreateSettings(100, 50, TransactionBondPolicy::Unbonded), true); // cache size > block max size
	}

	// endregion
}}
