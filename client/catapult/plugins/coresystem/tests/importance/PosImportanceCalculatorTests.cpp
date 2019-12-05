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

#include "src/importance/ImportanceCalculator.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/state/AccountActivityBuckets.h"
#include "tests/test/cache/AccountStateCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace importance {

#define TEST_CLASS PosImportanceCalculatorTests

	namespace {
		constexpr MosaicId Harvesting_Mosaic_Id(9876);
		constexpr model::ImportanceHeight Recalculation_Height(360);
		constexpr uint8_t Num_Account_States = 10;

		model::BlockChainConfiguration CreateBlockChainConfiguration(uint8_t importanceActivityPercentage) {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.ImportanceGrouping = 1;
			config.TotalChainImportance = Importance(9'000);
			config.ImportanceActivityPercentage = importanceActivityPercentage;
			config.MinHarvesterBalance = Amount(1'000'000);
			return config;
		}

		auto CreateActivityBucket(Amount fees, uint32_t beneficiaryCount, model::ImportanceHeight importanceHeight) {
			return state::AccountActivityBuckets::ActivityBucket{ { fees, beneficiaryCount, 0 }, importanceHeight };
		}

		void RecalculateTwice(
				ImportanceCalculator& calculator,
				model::ImportanceHeight importanceHeight,
				cache::AccountStateCacheDelta& delta) {
			// need to recalculate importance at two importance heights because importance is the minimum of two consecutive calculations
			calculator.recalculate(importanceHeight - model::ImportanceHeight(1), delta);
			calculator.recalculate(importanceHeight, delta);
		}

		struct AccountSeed {
		public:
			AccountSeed(catapult::Amount amount, const std::vector<state::AccountActivityBuckets::ActivityBucket>& buckets)
					: Amount(amount)
					, Buckets(buckets)
			{}

		public:
			catapult::Amount Amount;
			std::vector<state::AccountActivityBuckets::ActivityBucket> Buckets;
		};

		struct CacheHolder {
		public:
			explicit CacheHolder(Amount minBalance)
					: Cache(cache::CacheConfiguration(), CreateAccountStateCacheOptions(minBalance))
					, Delta(Cache.createDelta())
			{}

		public:
			void seedDelta(const std::vector<AccountSeed>& accountSeeds, model::ImportanceHeight importanceHeight) {
				uint8_t i = 0;
				for (auto accountData : accountSeeds) {
					auto key = Key{ { ++i } };
					Delta->addAccount(key, Height(importanceHeight.unwrap()));
					auto& accountState = Delta->find(key).get();
					accountState.Balances.credit(Harvesting_Mosaic_Id, accountData.Amount);
					for (const auto& dataBucket : accountData.Buckets) {
						accountState.ActivityBuckets.update(dataBucket.StartHeight, [&dataBucket](auto& bucket) {
							bucket = dataBucket;
						});
					}
				}
			}

		public:
			state::AccountState& get(const Key& publicKey) {
				return Delta->find(publicKey).get();
			}

		public:
			cache::AccountStateCache Cache;
			cache::LockedCacheDelta<cache::AccountStateCacheDelta> Delta;

		private:
			static cache::AccountStateCacheTypes::Options CreateAccountStateCacheOptions(Amount minBalance) {
				auto options = test::CreateDefaultAccountStateCacheOptions(MosaicId(1111), Harvesting_Mosaic_Id);
				options.MinHarvesterBalance = minBalance;
				return options;
			}
		};

		template<typename TTraits>
		void AssertCumulativeImportance(cache::AccountStateCacheDelta& cache) {
			// Act:
			size_t sum = 0;
			size_t maxExpectedDeviation = 0;
			auto config = TTraits::CreateConfiguration();
			for (uint8_t i = 1; i <= Num_Account_States; ++i) {
				const auto& accountState = cache.find(Key{ { i } }).get();
				sum += accountState.ImportanceSnapshots.current().unwrap();
				if (config.MinHarvesterBalance <= accountState.Balances.get(Harvesting_Mosaic_Id))
					++maxExpectedDeviation;
			}

			// Assert: deviation should be maximal 1 for each account due to rounding
			auto deviation = config.TotalChainImportance.unwrap() - sum;
			EXPECT_GE(maxExpectedDeviation, deviation);
		}

		template<typename TTraits>
		void AssertPosImportanceCalculatorRespectsCustomMinBalance(uint64_t minBalance, int64_t delta, bool hasNonzeroImportance) {
			// Arrange:
			auto config = TTraits::CreateConfiguration();

			std::vector<AccountSeed> accountSeeds;
			accountSeeds.push_back({ Amount(minBalance + static_cast<Amount::ValueType>(delta)), {} });
			accountSeeds.push_back({ Amount(minBalance), {} });

			CacheHolder holder((Amount(minBalance)));
			holder.seedDelta(accountSeeds, Recalculation_Height);
			auto pCalculator = CreateImportanceCalculator(config);

			// Act:
			RecalculateTwice(*pCalculator, Recalculation_Height, *holder.Delta);

			const auto& accountState1 = holder.get(Key{ { 1 } });
			const auto& accountState2 = holder.get(Key{ { 2 } });

			// Assert:
			EXPECT_EQ(hasNonzeroImportance, Importance() < accountState1.ImportanceSnapshots.current());
			EXPECT_LT(Importance(), accountState2.ImportanceSnapshots.current());
		}
	}

	struct ActivityTraits {
		static model::BlockChainConfiguration CreateConfiguration() {
			return CreateBlockChainConfiguration(10);
		}

		static void IncreaseImportance(state::AccountState& accountState) {
			accountState.ActivityBuckets.update(Recalculation_Height - model::ImportanceHeight(1), [](auto& bucket) {
				bucket.TotalFeesPaid = Amount(1000);
				bucket.BeneficiaryCount = 1000;
			});
		}
	};

	struct NoActivityTraits {
		static model::BlockChainConfiguration CreateConfiguration() {
			return CreateBlockChainConfiguration(0);
		}

		static void IncreaseImportance(state::AccountState& accountState) {
			accountState.Balances.credit(Harvesting_Mosaic_Id, Amount(1'000'000));
		}
	};

#define ACTIVITY_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_WithActivity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ActivityTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_WithoutActivity) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NoActivityTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ACTIVITY_BASED_TEST(PosImportanceCalculatorRespectsCustomMinBalance) {
		AssertPosImportanceCalculatorRespectsCustomMinBalance<TTraits>(1'000'000, -1, false);
		AssertPosImportanceCalculatorRespectsCustomMinBalance<TTraits>(1'000'000, 0, true);
		AssertPosImportanceCalculatorRespectsCustomMinBalance<TTraits>(1'000'000, 1, true);
	}

	ACTIVITY_BASED_TEST(AccountImportancesSumIsEqualToTotalChainImportance) {
		// Arrange:
		auto config = TTraits::CreateConfiguration();

		std::vector<AccountSeed> accountSeeds;
		for (auto i = 1u; i <= Num_Account_States; ++i) {
			auto amount = Amount(i * config.MinHarvesterBalance.unwrap());
			std::vector<state::AccountActivityBuckets::ActivityBucket> buckets;
			buckets.push_back(CreateActivityBucket(Amount(i * 20), i * 10, Recalculation_Height - model::ImportanceHeight(2)));
			buckets.push_back(CreateActivityBucket(Amount(i * 180), i * 90, Recalculation_Height - model::ImportanceHeight(1)));
			accountSeeds.emplace_back(amount, buckets);
		}

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(accountSeeds, Recalculation_Height);
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		RecalculateTwice(*pCalculator, Recalculation_Height, *holder.Delta);

		// Assert:
		AssertCumulativeImportance<TTraits>(*holder.Delta);
	}

	ACTIVITY_BASED_TEST(PosSetsImportanceToZeroWhenAccountBalanceIsBelowMinimum) {
		// Arrange:
		auto config = TTraits::CreateConfiguration();

		std::vector<AccountSeed> accountSeeds;
		for (auto i = 1u; i <= Num_Account_States; ++i) {
			auto amount = Amount(i * config.MinHarvesterBalance.unwrap() / 5);
			std::vector<state::AccountActivityBuckets::ActivityBucket> buckets;
			buckets.push_back(CreateActivityBucket(Amount(i * 20), i * 10, Recalculation_Height - model::ImportanceHeight(2)));
			buckets.push_back(CreateActivityBucket(Amount(i * 180), i * 90, Recalculation_Height - model::ImportanceHeight(1)));
			accountSeeds.emplace_back(amount, buckets);
		}

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(accountSeeds, Recalculation_Height);
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		RecalculateTwice(*pCalculator, Recalculation_Height, *holder.Delta);

		// Assert:
		AssertCumulativeImportance<TTraits>(*holder.Delta);
		for (uint8_t i = 1; i <= Num_Account_States; ++i) {
			// first four accounts have balance < MinHarvesterBalance
			const auto& accountState = holder.get(Key{ { i } });
			if (i < 5)
				EXPECT_EQ(Importance(), accountState.ImportanceSnapshots.current());
			else
				EXPECT_LT(Importance(), accountState.ImportanceSnapshots.current());
		}
	}

	ACTIVITY_BASED_TEST(ImportanceIsProportionalToTheAvailableTotalChainImportance) {
		// Arrange:
		auto config = TTraits::CreateConfiguration();

		std::vector<AccountSeed> accountSeeds;
		for (auto i = 1u; i <= Num_Account_States; ++i) {
			auto amount = Amount(config.MinHarvesterBalance.unwrap());
			std::vector<state::AccountActivityBuckets::ActivityBucket> buckets;
			buckets.push_back(CreateActivityBucket(Amount(i * 20), i * 10, Recalculation_Height - model::ImportanceHeight(4)));
			buckets.push_back(CreateActivityBucket(Amount(i * 180), i * 90, Recalculation_Height - model::ImportanceHeight(3)));
			accountSeeds.emplace_back(amount, buckets);
		}

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(accountSeeds, model::ImportanceHeight(1));
		const auto& accountState = holder.get(Key{ { 1 } });
		auto pCalculator1 = CreateImportanceCalculator(config);

		auto customConfig = TTraits::CreateConfiguration();
		customConfig.TotalChainImportance = Importance(2 * config.TotalChainImportance.unwrap());
		auto pCalculator2 = CreateImportanceCalculator(customConfig);

		// Act:
		RecalculateTwice(*pCalculator1, Recalculation_Height - model::ImportanceHeight(2), *holder.Delta);
		auto importance1 = accountState.ImportanceSnapshots.current();

		RecalculateTwice(*pCalculator2, Recalculation_Height, *holder.Delta);
		auto importance2 = accountState.ImportanceSnapshots.current();

		// Assert:
		EXPECT_EQ(importance1 + importance1, importance2);
	}

	ACTIVITY_BASED_TEST(CurrentImportanceIsMinimumOfLastTwoCalculations) {
		// Arrange:
		auto config = TTraits::CreateConfiguration();

		std::vector<AccountSeed> accountSeeds;
		std::vector<state::AccountActivityBuckets::ActivityBucket> buckets;
		accountSeeds.emplace_back(config.MinHarvesterBalance, buckets);
		accountSeeds.emplace_back(config.MinHarvesterBalance, buckets);

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(accountSeeds, model::ImportanceHeight(1));
		auto& accountState = holder.get(Key{ { 1 } });
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		pCalculator->recalculate(Recalculation_Height - model::ImportanceHeight(3), *holder.Delta);
		auto importance1 = accountState.ImportanceSnapshots.current();

		pCalculator->recalculate(Recalculation_Height - model::ImportanceHeight(2), *holder.Delta);
		auto importance2 = accountState.ImportanceSnapshots.current();

		// - increase importance of the account
		TTraits::IncreaseImportance(accountState);

		// - recalculate with increased importance
		pCalculator->recalculate(Recalculation_Height - model::ImportanceHeight(1), *holder.Delta);
		auto importance3 = accountState.ImportanceSnapshots.current();

		pCalculator->recalculate(Recalculation_Height, *holder.Delta);
		auto importance4 = accountState.ImportanceSnapshots.current();

		pCalculator->recalculate(Recalculation_Height + model::ImportanceHeight(10), *holder.Delta);
		auto importance5 = accountState.ImportanceSnapshots.current();

		// Assert:
		EXPECT_EQ(Importance(), importance1); // previous importance is 0
		EXPECT_LT(Importance(), importance2); // same consecutive importances
		EXPECT_EQ(importance2, importance3); // importance2 is minimum
		EXPECT_LT(importance3, importance4); // same consecutive importances
		EXPECT_EQ(Importance(), importance5); // previous importance is 0
	}

	ACTIVITY_BASED_TEST(ImportanceIsSetAtNemesisHeight) {
		// Arrange:
		auto config = TTraits::CreateConfiguration();

		std::vector<AccountSeed> accountSeeds;
		std::vector<state::AccountActivityBuckets::ActivityBucket> buckets;
		accountSeeds.emplace_back(config.MinHarvesterBalance, buckets);
		accountSeeds.emplace_back(config.MinHarvesterBalance, buckets);

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(accountSeeds, model::ImportanceHeight(1));
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		pCalculator->recalculate(model::ImportanceHeight(1), *holder.Delta);

		// Assert:
		EXPECT_LT(Importance(), holder.get(Key{ { 1 } }).ImportanceSnapshots.current());
		EXPECT_LT(Importance(), holder.get(Key{ { 2 } }).ImportanceSnapshots.current());
	}

	// region pure pos

	TEST(TEST_CLASS, PosGivesAccountsImportanceProportionalToBalance) {
		// Arrange:
		auto config = CreateBlockChainConfiguration(0);

		std::vector<AccountSeed> accountSeeds;
		for (auto i = 1u; i <= Num_Account_States; ++i)
			accountSeeds.push_back({ Amount(i * i * config.MinHarvesterBalance.unwrap()), {} });

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(accountSeeds, Recalculation_Height);
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		RecalculateTwice(*pCalculator, Recalculation_Height, *holder.Delta);

		// Assert:
		const auto& referenceAccountState = holder.get(Key{ { 1 } });
		auto referenceAmount = referenceAccountState.Balances.get(Harvesting_Mosaic_Id).unwrap();
		auto referenceImportance = referenceAccountState.ImportanceSnapshots.current().unwrap();
		for (uint8_t i = 1; i <= Num_Account_States; ++i) {
			// deviation should be maximal i * i due to rounding
			const auto& accountState = holder.get(Key{ { i } });
			EXPECT_EQ(Amount(referenceAmount * i * i), accountState.Balances.get(Harvesting_Mosaic_Id));
			EXPECT_GE(Importance((referenceImportance + 1) * i * i), accountState.ImportanceSnapshots.current());
			EXPECT_LE(Importance(referenceImportance * i * i), accountState.ImportanceSnapshots.current());
			EXPECT_EQ(Recalculation_Height, accountState.ImportanceSnapshots.height());
		}
	}

	// endregion

	// region pos + activity

	namespace {
		template<typename TCreateBucket>
		void AssertActivityIncreasesImportance(TCreateBucket createBucket) {
			// Arrange:
			auto config = CreateBlockChainConfiguration(10);

			std::vector<AccountSeed> accountSeeds;
			auto amount = Amount(config.MinHarvesterBalance.unwrap());
			for (auto i = 0u; i <= Num_Account_States; ++i) {
				std::vector<state::AccountActivityBuckets::ActivityBucket> buckets;
				buckets.push_back(createBucket(i));
				accountSeeds.emplace_back(amount, buckets);
			}

			CacheHolder holder(config.MinHarvesterBalance);
			holder.seedDelta(accountSeeds, model::ImportanceHeight(1));
			auto pCalculator = CreateImportanceCalculator(config);

			// Act:
			RecalculateTwice(*pCalculator, Recalculation_Height, *holder.Delta);

			// Assert:
			Importance currentImportance;
			for (uint8_t i = 1; i <= Num_Account_States; ++i) {
				auto importance = holder.get(Key{ { i } }).ImportanceSnapshots.current();
				EXPECT_LT(currentImportance, importance);
				currentImportance = importance;
			}
		}
	}

	TEST(TEST_CLASS, FeeActivityIncreasesImportance) {
		// Act:
		AssertActivityIncreasesImportance([](auto i) {
			return CreateActivityBucket(Amount(i * 1000), 0, Recalculation_Height - model::ImportanceHeight(1));
		});
	}

	TEST(TEST_CLASS, BeneficiaryActivityIncreasesImportance) {
		// Act:
		AssertActivityIncreasesImportance([](auto i) {
			return CreateActivityBucket(Amount(), i * 1000, Recalculation_Height - model::ImportanceHeight(1));
		});
	}

	TEST(TEST_CLASS, FeeActivityIncreasesImportanceMoreThanBeneficiaryActivity) {
		// Arrange:
		auto config = CreateBlockChainConfiguration(10);

		std::vector<AccountSeed> accountSeeds;
		std::vector<state::AccountActivityBuckets::ActivityBucket> buckets1;
		buckets1.push_back(CreateActivityBucket(Amount(1000), 0, Recalculation_Height - model::ImportanceHeight(1)));
		accountSeeds.emplace_back(config.MinHarvesterBalance, buckets1);

		std::vector<state::AccountActivityBuckets::ActivityBucket> buckets2;
		buckets2.push_back(CreateActivityBucket(Amount(), 1000, Recalculation_Height - model::ImportanceHeight(1)));
		accountSeeds.emplace_back(config.MinHarvesterBalance, buckets2);

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(accountSeeds, model::ImportanceHeight(1));
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		RecalculateTwice(*pCalculator, Recalculation_Height, *holder.Delta);

		// Assert:
		auto importance1 = holder.get(Key{ { 1 } }).ImportanceSnapshots.current();
		auto importance2 = holder.get(Key{ { 2 } }).ImportanceSnapshots.current();
		EXPECT_GT(importance1, importance2);
	}

	TEST(TEST_CLASS, ActivityBasedImportanceDecreasesWithIncreasingStake) {
		// Arrange:
		auto config = CreateBlockChainConfiguration(10);

		std::vector<AccountSeed> accountSeeds;
		for (auto i = 1u; i <= 2; ++i) {
			std::vector<state::AccountActivityBuckets::ActivityBucket> buckets1;
			buckets1.push_back(CreateActivityBucket(Amount(), 0, Recalculation_Height - model::ImportanceHeight(1)));
			accountSeeds.emplace_back(Amount(i * config.MinHarvesterBalance.unwrap()), buckets1);

			std::vector<state::AccountActivityBuckets::ActivityBucket> buckets2;
			buckets2.push_back(CreateActivityBucket(Amount(1000), 1000, Recalculation_Height - model::ImportanceHeight(1)));
			accountSeeds.emplace_back(Amount(i * config.MinHarvesterBalance.unwrap()), buckets2);
		}

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(accountSeeds, model::ImportanceHeight(1));
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		RecalculateTwice(*pCalculator, Recalculation_Height, *holder.Delta);

		// Assert:
		std::array<Importance, 4> importances;
		for (uint8_t i = 1u; i <= importances.size(); ++i)
			importances[i - 1] = holder.get(Key{ { i } }).ImportanceSnapshots.current();

		auto activityImportance1 = importances[1] - importances[0];
		auto activityImportance2 = importances[3] - importances[2];
		EXPECT_GT(activityImportance1, activityImportance2);
	}

	// endregion

	// region removed accounts

	TEST(TEST_CLASS, PosDisablesActivityCollectionAndRemovesBucketWhenPresent) {
		// Arrange:
		auto config = CreateBlockChainConfiguration(0);

		std::vector<AccountSeed> accountSeeds;
		for (auto i = 1u; i <= Num_Account_States; ++i) {
			std::vector<state::AccountActivityBuckets::ActivityBucket> buckets;
			buckets.push_back(CreateActivityBucket(Amount(100), 200, Recalculation_Height));
			accountSeeds.push_back({ config.MinHarvesterBalance, buckets });
		}

		// Sanity:
		EXPECT_EQ(10u, accountSeeds.size());

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(accountSeeds, Recalculation_Height);
		holder.Cache.commit();
		for (uint8_t i = 1u; i <= Num_Account_States; ++i) {
			auto& accountState = holder.get(Key{ { i } });
			if (1 == i % 2) {
				accountState.Balances.debit(Harvesting_Mosaic_Id, Amount(1));
				if (1 == i % 4)
					holder.Delta->queueRemove(accountState.PublicKey, accountState.PublicKeyHeight);
			}
		}

		holder.Delta->commitRemovals();
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		pCalculator->recalculate(Recalculation_Height, *holder.Delta);

		// Assert:
		for (uint8_t i = 1u; i <= Num_Account_States; ++i) {
			if (1 == i % 4) {
				// note that this will not happen in real scenario
				EXPECT_FALSE(holder.Delta->contains(Key{ { i } })) << "account " << i;
			} else {
				const auto& buckets = holder.Delta->find(Key{ { i } }).get().ActivityBuckets;
				if (1 == i % 2)
					EXPECT_EQ(model::ImportanceHeight(), buckets.get(Recalculation_Height).StartHeight) << "account " << i;
				else
					EXPECT_EQ(Recalculation_Height, buckets.get(Recalculation_Height).StartHeight) << "account " << i;
			}
		}
	}

	// endregion
}}
