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

#include "harvesting/src/HarvestingService.h"
#include "harvesting/src/HarvestingConfiguration.h"
#include "harvesting/src/UnlockedAccounts.h"
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvestingServiceTests

	namespace {
		constexpr auto Service_Name = "unlockedAccounts";
		constexpr auto Task_Name = "harvesting task";
		constexpr auto Harvester_Key = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";

		HarvestingConfiguration CreateHarvestingConfiguration(bool autoHarvest) {
			return HarvestingConfiguration{ Harvester_Key, autoHarvest, 10 };
		}

		struct HarvestingServiceTraits {
			static auto CreateRegistrar(const HarvestingConfiguration& config) {
				return CreateHarvestingServiceRegistrar(config);
			}

			static auto CreateRegistrar() {
				return CreateRegistrar(HarvestingConfiguration::Uninitialized());
			}
		};

		class TestContext : public test::ServiceLocatorTestContext<HarvestingServiceTraits> {
		private:
			using BaseType = test::ServiceLocatorTestContext<HarvestingServiceTraits>;

		public:
			explicit TestContext(test::LocalNodeFlags flags = test::LocalNodeFlags::None)
					: m_config(CreateHarvestingConfiguration(test::LocalNodeFlags::Should_Auto_Harvest == flags)) {
				setHooks();
			}

			explicit TestContext(cache::CatapultCache&& cache, const supplier<Timestamp>& timeSupplier = &utils::NetworkTime)
					: BaseType(std::move(cache), timeSupplier)
					, m_config(CreateHarvestingConfiguration(false)) {
				setHooks();
			}

		public:
			Key harvesterKey() const {
				return crypto::KeyPair::FromString(m_config.HarvestKey).publicKey();
			}

			const auto& capturedStateHashes() const {
				return m_capturedStateHashes;
			}

		public:
			void setMinHarvesterBalance(Amount balance) {
				const_cast<model::BlockChainConfiguration&>(testState().state().config().BlockChain).MinHarvesterBalance = balance;
			}

			void enableVerifiableState() {
				auto& config = testState().state().config();
				const_cast<bool&>(config.Node.ShouldUseCacheDatabaseStorage) = true;
				const_cast<bool&>(config.BlockChain.ShouldEnableVerifiableState) = true;
			}

		public:
			void boot() {
				ServiceLocatorTestContext::boot(m_config);
			}

		private:
			void setHooks() {
				// set up hooks
				auto& capturedStateHashes = m_capturedStateHashes;
				testState().state().hooks().setCompletionAwareBlockRangeConsumerFactory([&capturedStateHashes](auto) {
					return [&capturedStateHashes](auto&& blockRange, auto) {
						for (const auto& block : blockRange)
							capturedStateHashes.push_back(block.StateHash);

						return disruptor::DisruptorElementId();
					};
				});
			}

		private:
			HarvestingConfiguration m_config;
			std::vector<Hash256> m_capturedStateHashes;
		};

		std::shared_ptr<UnlockedAccounts> GetUnlockedAccounts(const extensions::ServiceLocator& locator) {
			return locator.service<UnlockedAccounts>(Service_Name);
		}
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(Harvesting, Post_Range_Consumers)

	// region unlocked accounts

	namespace {
		template<typename TAction>
		void RunUnlockedAccountsServiceTest(test::LocalNodeFlags localNodeFlags, TAction action) {
			// Arrange:
			TestContext context(localNodeFlags);

			// Act:
			context.boot();

			// Assert:
			EXPECT_EQ(1u, context.locator().numServices());
			EXPECT_EQ(1u, context.locator().counters().size());

			auto pUnlockedAccounts = GetUnlockedAccounts(context.locator());
			ASSERT_TRUE(!!pUnlockedAccounts);
			action(*pUnlockedAccounts, context);
		}
	}

	TEST(TEST_CLASS, UnlockedAccountsServiceIsRegisteredProperlyWhenAutoHarvestingIsEnabled) {
		// Arrange:
		RunUnlockedAccountsServiceTest(test::LocalNodeFlags::Should_Auto_Harvest, [](const auto& accounts, const auto& context) {
			// Assert: a single account was unlocked
			EXPECT_TRUE(accounts.view().contains(context.harvesterKey()));
			EXPECT_EQ(1u, context.counter("UNLKED ACCTS"));
		});
	}

	TEST(TEST_CLASS, UnlockedAccountsServiceIsRegisteredProperlyWhenAutoHarvestingIsDisabled) {
		// Arrange:
		RunUnlockedAccountsServiceTest(test::LocalNodeFlags::None, [](const auto& accounts, const auto& context) {
			// Assert: no accounts were unlocked
			EXPECT_FALSE(accounts.view().contains(context.harvesterKey()));
			EXPECT_EQ(0u, context.counter("UNLKED ACCTS"));
		});
	}

	// endregion

	// region harvesting task - utils + basic

	namespace {
		template<typename TAction>
		void RunTaskTest(TestContext& context, const std::string& taskName, TAction&& action) {
			// Act:
			test::RunTaskTest(context, 1, taskName, [&context, action = std::move(action)](const auto& task) mutable {
				auto pUnlockedAccounts = GetUnlockedAccounts(context.locator());
				ASSERT_TRUE(!!pUnlockedAccounts);
				action(*pUnlockedAccounts, task);
			});
		}
	}

	namespace {
		constexpr Amount Account_Balance(1000);
		constexpr auto Importance_Grouping = 234u;

		auto ConvertToImportanceHeight(Height height) {
			return model::ConvertToImportanceHeight(height, Importance_Grouping);
		}

		auto CreateCacheWithAccount(
				const cache::CacheConfiguration& cacheConfig,
				Height height,
				const Key& publicKey,
				Amount balance,
				model::ImportanceHeight importanceHeight) {
			// Arrange:
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.ImportanceGrouping = Importance_Grouping;
			auto cache = test::CreateEmptyCatapultCache(config, cacheConfig);
			auto delta = cache.createDelta();

			// - add an account
			auto& accountStateCache = delta.sub<cache::AccountStateCache>();
			accountStateCache.addAccount(publicKey, Height(100));
			auto& accountState = accountStateCache.find(publicKey).get();
			accountState.ImportanceInfo.set(Importance(123), importanceHeight);
			accountState.Balances.credit(Xem_Id, balance);

			// - add a block difficulty info
			auto& blockDifficultyCache = delta.sub<cache::BlockDifficultyCache>();
			blockDifficultyCache.insert(state::BlockDifficultyInfo(Height(1)));

			// - commit changes
			cache.commit(height);
			return cache;
		}

		auto CreateCacheWithAccount(Height height, const Key& publicKey, Amount balance, model::ImportanceHeight importanceHeight) {
			return CreateCacheWithAccount(cache::CacheConfiguration(), height, publicKey, balance, importanceHeight);
		}
	}

	TEST(TEST_CLASS, HarvestingTaskIsScheduled) {
		// Assert:
		test::AssertRegisteredTask(TestContext(), 1, Task_Name);
	}

	// endregion

	// region harvesting task - pruning

	TEST(TEST_CLASS, HarvestingTaskDoesNotPruneEligibleAccount) {
		// Arrange: eligible because next height and importance height match
		auto height = Height(2 * Importance_Grouping - 1);
		auto importanceHeight = model::ImportanceHeight(Importance_Grouping);
		auto keyPair = test::GenerateKeyPair();
		TestContext context(CreateCacheWithAccount(height, keyPair.publicKey(), Account_Balance, importanceHeight));
		context.setMinHarvesterBalance(Account_Balance);

		// Sanity:
		EXPECT_EQ(importanceHeight, ConvertToImportanceHeight(height + Height(1)));

		RunTaskTest(context, Task_Name, [keyPair = std::move(keyPair)](auto& unlockedAccounts, const auto& task) mutable {
			unlockedAccounts.modifier().add(std::move(keyPair));

			// Act:
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
			EXPECT_EQ(1u, unlockedAccounts.view().size());
		});
	}

	TEST(TEST_CLASS, HarvestingTaskDoesPruneAccountIneligibleDueToImportanceHeight) {
		// Arrange: ineligible because next height and importance height do not match
		auto height = Height(2 * Importance_Grouping);
		auto importanceHeight = model::ImportanceHeight(Importance_Grouping);
		auto keyPair = test::GenerateKeyPair();
		TestContext context(CreateCacheWithAccount(height, keyPair.publicKey(), Account_Balance, importanceHeight));
		context.setMinHarvesterBalance(Account_Balance);

		// Sanity:
		EXPECT_NE(importanceHeight, ConvertToImportanceHeight(height + Height(1)));

		RunTaskTest(context, Task_Name, [keyPair = std::move(keyPair)](auto& unlockedAccounts, const auto& task) mutable {
			unlockedAccounts.modifier().add(std::move(keyPair));

			// Act:
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
			EXPECT_EQ(0u, unlockedAccounts.view().size());
		});
	}

	TEST(TEST_CLASS, HarvestingTaskDoesPruneAccountIneligibleDueToBalance) {
		// Arrange: ineligible because account balance is too low
		auto height = Height(2 * Importance_Grouping - 1);
		auto importanceHeight = model::ImportanceHeight(Importance_Grouping);
		auto keyPair = test::GenerateKeyPair();
		TestContext context(CreateCacheWithAccount(height, keyPair.publicKey(), Account_Balance, importanceHeight));
		context.setMinHarvesterBalance(Account_Balance + Amount(1));

		// Sanity:
		EXPECT_EQ(importanceHeight, ConvertToImportanceHeight(height + Height(1)));

		RunTaskTest(context, Task_Name, [keyPair = std::move(keyPair)](auto& unlockedAccounts, const auto& task) mutable {
			unlockedAccounts.modifier().add(std::move(keyPair));

			// Act:
			auto result = task.Callback().get();

			// Assert:
			EXPECT_EQ(thread::TaskResult::Continue, result);
			EXPECT_EQ(0u, unlockedAccounts.view().size());
		});
	}

	// endregion

	// region harvesting task - state hash

	namespace {
		void RunHarvestingStateHashTest(bool enableVerifiableState, Hash256& harvestedStateHash) {
			// Arrange: use a huge amount and a max timestamp to force a hit
			test::TempDirectoryGuard dbDirGuard("testdb");
			auto keyPair = test::GenerateKeyPair();
			auto balance = Amount(1'000'000'000'000);
			auto cacheConfig = enableVerifiableState
					? cache::CacheConfiguration(dbDirGuard.name(), utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled)
					: cache::CacheConfiguration();
			TestContext context(
					CreateCacheWithAccount(cacheConfig, Height(1), keyPair.publicKey(), balance, model::ImportanceHeight(1)),
					[]() { return Timestamp(std::numeric_limits<int64_t>::max()); });
			if (enableVerifiableState)
				context.enableVerifiableState();

			RunTaskTest(context, Task_Name, [keyPair = std::move(keyPair), &context, &harvestedStateHash](
					auto& unlockedAccounts,
					const auto& task) mutable {
				unlockedAccounts.modifier().add(std::move(keyPair));

				// Act:
				auto result = task.Callback().get();

				// Assert: one block should have been harvested
				ASSERT_EQ(1u, context.capturedStateHashes().size());
				harvestedStateHash = context.capturedStateHashes()[0];

				// Sanity:
				EXPECT_EQ(thread::TaskResult::Continue, result);
				EXPECT_EQ(1u, unlockedAccounts.view().size());
			});
		}
	}

	TEST(TEST_CLASS, HarvestingTaskGeneratesZeroStateHashWhenVerifiableStateIsDisabled) {
		// Act:
		Hash256 harvestedStateHash;
		RunHarvestingStateHashTest(false, harvestedStateHash);

		// Assert:
		EXPECT_EQ(Hash256(), harvestedStateHash);
	}

	TEST(TEST_CLASS, HarvestingTaskGeneratesNonZeroStateHashWhenVerifiableStateIsEnabled) {
		// Act:
		Hash256 harvestedStateHash;
		RunHarvestingStateHashTest(true, harvestedStateHash);

		// Assert:
		EXPECT_NE(Hash256(), harvestedStateHash);
	}

	// endregion
}}
