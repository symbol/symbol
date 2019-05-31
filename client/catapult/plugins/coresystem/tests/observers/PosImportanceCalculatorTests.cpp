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

#include "src/observers/ImportanceCalculator.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/NetworkInfo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS PosImportanceCalculatorTests

	namespace {
		constexpr MosaicId Harvesting_Mosaic_Id(9876);
		constexpr model::ImportanceHeight Recalculation_Height(360);
		constexpr uint8_t Num_Account_States = 10;

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.TotalChainImportance = Importance(9'000);
			config.MinHarvesterBalance = Amount(1'000'000'000'000);
			return config;
		}

		struct CacheHolder {
		public:
			explicit CacheHolder(Amount minBalance)
					: Cache(cache::CacheConfiguration(), {
						model::NetworkIdentifier::Mijin_Test,
						123,
						minBalance,
						MosaicId(1111),
						Harvesting_Mosaic_Id
					})
					, Delta(Cache.createDelta())
			{}

		public:
			void seedDelta(const std::vector<Amount::ValueType>& amounts, model::ImportanceHeight height) {
				uint8_t i = 0;
				for (auto amount : amounts) {
					auto key = Key{ { ++i } };
					Delta->addAccount(key, Height(height.unwrap()));
					auto& accountState = Delta->find(key).get();
					accountState.Balances.credit(Harvesting_Mosaic_Id, Amount(amount));
				}
			}

		public:
			const state::AccountState& get(const Key& publicKey) {
				return Delta->find(publicKey).get();
			}

		public:
			cache::AccountStateCache Cache;
			cache::LockedCacheDelta<cache::AccountStateCacheDelta> Delta;
		};

		void AssertCumulativeImportance(cache::AccountStateCacheDelta& cache) {
			// Act:
			size_t sum = 0;
			size_t maxExpectedDeviation = 0;
			auto config = CreateConfiguration();
			for (uint8_t i = 1; i <= Num_Account_States; ++i) {
				const auto& accountState = cache.find(Key{ { i } }).get();
				sum += accountState.ImportanceInfo.current().unwrap();
				if (config.MinHarvesterBalance <= accountState.Balances.get(Harvesting_Mosaic_Id))
					++maxExpectedDeviation;
			}

			// Assert: deviation should be maximal 1 for each account due to rounding
			auto deviation = config.TotalChainImportance.unwrap() - sum;
			EXPECT_GE(maxExpectedDeviation, deviation);
		}

		void AssertPosImportanceCalculatorRespectsCustomMinBalance(uint64_t minBalance, int64_t delta, bool hasNonZeroImportance) {
			// Arrange:
			auto config = CreateConfiguration();

			std::vector<Amount::ValueType> amounts;
			amounts.push_back(minBalance + static_cast<Amount::ValueType>(delta));
			amounts.push_back(minBalance);

			CacheHolder holder((Amount(minBalance)));
			holder.seedDelta(amounts, Recalculation_Height);
			auto pCalculator = CreateImportanceCalculator(config);

			// Act:
			pCalculator->recalculate(Recalculation_Height, *holder.Delta);
			const auto& accountState1 = holder.get(Key{ { 1 } });
			const auto& accountState2 = holder.get(Key{ { 2 } });

			// Assert:
			EXPECT_EQ(hasNonZeroImportance, Importance() < accountState1.ImportanceInfo.current());
			EXPECT_LT(Importance(), accountState2.ImportanceInfo.current());
		}
	}

	TEST(TEST_CLASS, PosImportanceCalculatorRespectsCustomMinBalance) {
		// Assert:
		AssertPosImportanceCalculatorRespectsCustomMinBalance(1'000'000, -1, false);
		AssertPosImportanceCalculatorRespectsCustomMinBalance(1'000'000, 0, true);
		AssertPosImportanceCalculatorRespectsCustomMinBalance(1'000'000, 1, true);
	}

	TEST(TEST_CLASS, PosGivesAccountsImportanceProportionalToBalance) {
		// Arrange:
		auto config = CreateConfiguration();
		std::vector<Amount::ValueType> amounts;
		for (auto i = 1u; i <= Num_Account_States; ++i)
			amounts.push_back(i * i * config.MinHarvesterBalance.unwrap());

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(amounts, Recalculation_Height);
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		pCalculator->recalculate(Recalculation_Height, *holder.Delta);

		// Assert:
		const auto& referenceAccountState = holder.get(Key{ { 1 } });
		auto referenceAmount = referenceAccountState.Balances.get(Harvesting_Mosaic_Id).unwrap();
		auto referenceImportance = referenceAccountState.ImportanceInfo.current().unwrap();
		for (uint8_t i = 1; i <= Num_Account_States; ++i) {
			// deviation should be maximal i * i due to rounding
			const auto& accountState = holder.get(Key{ { i } });
			EXPECT_EQ(Amount(referenceAmount * i * i), accountState.Balances.get(Harvesting_Mosaic_Id));
			EXPECT_GE(Importance((referenceImportance + 1) * i * i), accountState.ImportanceInfo.current());
			EXPECT_LE(Importance(referenceImportance * i * i), accountState.ImportanceInfo.current());
			EXPECT_EQ(Recalculation_Height, accountState.ImportanceInfo.height());
		}
	}

	TEST(TEST_CLASS, AccountImportancesSumIsEqualToTotalChainImportance) {
		// Arrange:
		auto config = CreateConfiguration();
		std::vector<Amount::ValueType> amounts;
		for (auto i = 1u; i <= Num_Account_States; ++i)
			amounts.push_back(i * config.MinHarvesterBalance.unwrap());

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(amounts, Recalculation_Height);
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		pCalculator->recalculate(Recalculation_Height, *holder.Delta);

		// Assert:
		AssertCumulativeImportance(*holder.Delta);
	}

	TEST(TEST_CLASS, PosSetsImportanceToZeroWhenAccountBalanceIsBelowMinimum) {
		// Arrange:
		auto config = CreateConfiguration();
		std::vector<Amount::ValueType> amounts;
		for (auto i = 1u; i <= Num_Account_States; ++i)
			amounts.push_back(i * config.MinHarvesterBalance.unwrap() / 5);

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(amounts, Recalculation_Height);
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		pCalculator->recalculate(Recalculation_Height, *holder.Delta);

		// Assert:
		AssertCumulativeImportance(*holder.Delta);
		for (uint8_t i = 1; i <= Num_Account_States; ++i) {
			const auto& accountState = holder.get(Key{ { i } });
			if (i < 5)
				EXPECT_EQ(Importance(), accountState.ImportanceInfo.current());
			else
				EXPECT_LT(Importance(), accountState.ImportanceInfo.current());
		}
	}

	TEST(TEST_CLASS, ImportanceIsProportionalToTheAvailableTotalChainImportance) {
		// Arrange:
		auto config = CreateConfiguration();

		std::vector<Amount::ValueType> amounts;
		amounts.push_back(config.MinHarvesterBalance.unwrap());

		CacheHolder holder(config.MinHarvesterBalance);
		holder.seedDelta(amounts, model::ImportanceHeight(1));
		const auto& accountState = holder.get(Key{ { 1 } });
		auto pCalculator1 = CreateImportanceCalculator(config);

		auto customConfig = CreateConfiguration();
		customConfig.TotalChainImportance = Importance(2 * config.TotalChainImportance.unwrap());
		auto pCalculator2 = CreateImportanceCalculator(customConfig);

		// Act:
		pCalculator1->recalculate(model::ImportanceHeight(1), *holder.Delta);
		auto importance1 = accountState.ImportanceInfo.current();
		pCalculator2->recalculate(Recalculation_Height, *holder.Delta);
		auto importance2 = accountState.ImportanceInfo.current();

		// Assert:
		EXPECT_EQ(importance1 + importance1, importance2);
	}
}}
