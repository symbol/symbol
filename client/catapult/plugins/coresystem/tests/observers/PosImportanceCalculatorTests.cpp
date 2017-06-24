#include "src/observers/ImportanceCalculator.h"
#include "src/cache/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/NetworkInfo.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

	namespace {
		constexpr model::ImportanceHeight Recalculation_Height(360);
		constexpr uint8_t Num_Account_States = 10;
		constexpr uint64_t Microxem_Per_Xem = 1'000'000;

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.TotalChainBalance = Amount(9'000'000'000'000'000);
			config.MinHarvesterBalance = Amount(1'000'000'000'000);
			return config;
		}

		struct CacheHolder {
		public:
			CacheHolder()
					: Cache(model::NetworkIdentifier::Mijin_Test, 123)
					, Delta(Cache.createDelta())
			{}

		public:
			void seedDelta(const std::vector<Amount::ValueType>& amounts, model::ImportanceHeight height) {
				uint8_t i = 0;
				for (auto amount : amounts) {
					auto pState = Delta->addAccount(Key{ { ++i } }, Height(height.unwrap()));
					pState->Balances.credit(Xem_Id, Amount(amount));
				}
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
				auto pState = cache.findAccount(Key{ { i } });
				sum += pState->ImportanceInfo.current().unwrap();
				if (config.MinHarvesterBalance <= pState->Balances.get(Xem_Id))
					++maxExpectedDeviation;
			}

			// deviation should be maximal 1 for each account due to rounding
			auto deviation = config.TotalChainBalance.unwrap() / Microxem_Per_Xem - sum;
			EXPECT_GE(maxExpectedDeviation, deviation);
		}

		void AssertPosImportanceCalculatorRespectsCustomMinBalance(
				uint64_t minBalance,
				int64_t delta,
				bool hasNonZeroImportance) {
			// Arrange:
			auto config = CreateConfiguration();
			config.MinHarvesterBalance = Amount(minBalance);

			std::vector<Amount::ValueType> amounts;
			amounts.push_back(minBalance + static_cast<Amount::ValueType>(delta));
			amounts.push_back(minBalance);

			CacheHolder holder;
			holder.seedDelta(amounts, Recalculation_Height);
			auto pCalculator = CreateImportanceCalculator(config);

			// Act:
			pCalculator->recalculate(Recalculation_Height, *holder.Delta);
			auto pState1 = holder.Delta->findAccount(Key{ { 1 } });
			auto pState2 = holder.Delta->findAccount(Key{ { 2 } });

			// Assert:
			EXPECT_EQ(hasNonZeroImportance, 0u < pState1->ImportanceInfo.current().unwrap());
			EXPECT_LT(0u, pState2->ImportanceInfo.current().unwrap());
		}
	}

	TEST(PosImportanceCalculatorTests, PosImportanceCalculatorRespectsCustomMinBalance) {
		// Assert:
		AssertPosImportanceCalculatorRespectsCustomMinBalance(1'000'000, -1, false);
		AssertPosImportanceCalculatorRespectsCustomMinBalance(1'000'000, 0, true);
		AssertPosImportanceCalculatorRespectsCustomMinBalance(1'000'000, 1, true);
	}

	TEST(PosImportanceCalculatorTests, PosGivesAccountsImportanceProportionalToBalance) {
		// Arrange:
		auto config = CreateConfiguration();
		std::vector<Amount::ValueType> amounts;
		for (auto i = 1u; i <= Num_Account_States; ++i)
			amounts.push_back(i * i * config.MinHarvesterBalance.unwrap());

		CacheHolder holder;
		holder.seedDelta(amounts, Recalculation_Height);
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		pCalculator->recalculate(Recalculation_Height, *holder.Delta);

		// Assert:
		auto pAccountState = holder.Delta->findAccount(Key{ { 1 } });
		auto referenceAmount = pAccountState->Balances.get(Xem_Id).unwrap();
		auto referenceImportance = pAccountState->ImportanceInfo.current().unwrap();
		for (uint8_t i = 1; i <= Num_Account_States; ++i) {
			// deviation should be maximal i * i due to rounding
			auto pState = holder.Delta->findAccount(Key{ { i } });
			EXPECT_EQ(referenceAmount * i * i, pState->Balances.get(Xem_Id).unwrap());
			EXPECT_GE((referenceImportance + 1) * i * i, pState->ImportanceInfo.current().unwrap());
			EXPECT_LE(referenceImportance * i * i, pState->ImportanceInfo.current().unwrap());
			EXPECT_EQ(Recalculation_Height, pState->ImportanceInfo.height());
		}
	}

	TEST(PosImportanceCalculatorTests, AccountImportancesSumIsEqualToAllXem) {
		// Arrange:
		auto config = CreateConfiguration();
		std::vector<Amount::ValueType> amounts;
		for (auto i = 1u; i <= Num_Account_States; ++i)
			amounts.push_back(i * config.MinHarvesterBalance.unwrap());

		CacheHolder holder;
		holder.seedDelta(amounts, Recalculation_Height);
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		pCalculator->recalculate(Recalculation_Height, *holder.Delta);

		// Assert:
		AssertCumulativeImportance(*holder.Delta);
	}

	TEST(PosImportanceCalculatorTests, PosSetsImportanceToZeroIfAccountBalanceIsBelowMinimum) {
		// Arrange:
		auto config = CreateConfiguration();
		std::vector<Amount::ValueType> amounts;
		for (auto i = 1u; i <= Num_Account_States; ++i)
			amounts.push_back(i * config.MinHarvesterBalance.unwrap() / 5);

		CacheHolder holder;
		holder.seedDelta(amounts, Recalculation_Height);
		auto pCalculator = CreateImportanceCalculator(config);

		// Act:
		pCalculator->recalculate(Recalculation_Height, *holder.Delta);

		// Assert:
		AssertCumulativeImportance(*holder.Delta);
		for (uint8_t i = 1; i <= Num_Account_States; ++i) {
			auto pState = holder.Delta->findAccount(Key{ { i } });
			if (i < 5)
				EXPECT_EQ(0u, pState->ImportanceInfo.current().unwrap());
			else
				EXPECT_LT(0u, pState->ImportanceInfo.current().unwrap());
		}
	}

	TEST(PosImportanceCalculatorTests, ImportanceIsProportionalToTheAvailableXem) {
		// Arrange:
		auto config = CreateConfiguration();

		std::vector<Amount::ValueType> amounts;
		amounts.push_back(config.MinHarvesterBalance.unwrap());

		CacheHolder holder;
		holder.seedDelta(amounts, model::ImportanceHeight(1));
		auto pState = holder.Delta->findAccount(Key{ { 1 } });
		auto pCalculator1 = CreateImportanceCalculator(config);

		auto customConfig = CreateConfiguration();
		customConfig.TotalChainBalance = Amount(2 * config.TotalChainBalance.unwrap());
		auto pCalculator2 = CreateImportanceCalculator(customConfig);

		// Act:
		pCalculator1->recalculate(model::ImportanceHeight(1), *holder.Delta);
		auto importance1 = pState->ImportanceInfo.current();
		pCalculator2->recalculate(Recalculation_Height, *holder.Delta);
		auto importance2 = pState->ImportanceInfo.current();

		// Assert:
		EXPECT_EQ(importance1 + importance1, importance2);
	}
}}
