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
#include "harvesting/src/UnlockedAccountsStorage.h"
#include "catapult/cache_core/BlockStatisticCache.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "harvesting/tests/test/UnlockedTestEntry.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/HandlersTrustedHostTests.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/Functional.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvestingServiceTests

	namespace {
		constexpr auto Service_Name = "unlockedAccounts";
		constexpr auto Task_Name = "harvesting task";
		constexpr auto Harvester_Private_Key = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";

		HarvestingConfiguration CreateHarvestingConfiguration(test::LocalNodeFlags flags = test::LocalNodeFlags::None) {
			auto config = HarvestingConfiguration::Uninitialized();
			config.HarvesterPrivateKey = Harvester_Private_Key;
			config.EnableAutoHarvesting = test::LocalNodeFlags::Should_Auto_Harvest == flags;
			config.MaxUnlockedAccounts = 10;
			config.BeneficiaryPublicKey = std::string(64, '0');
			return config;
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
					: TestContext(CreateHarvestingConfiguration(flags))
			{}

			explicit TestContext(const HarvestingConfiguration& config)
					: BaseType(test::CreateEmptyCatapultCache(test::CreatePrototypicalBlockChainConfiguration()))
					, m_config(config) {
				setHooks();
			}

			explicit TestContext(
					cache::CatapultCache&& cache,
					const supplier<Timestamp>& timeSupplier = test::CreateDefaultNetworkTimeSupplier())
					: BaseType(std::move(cache), timeSupplier)
					, m_config(CreateHarvestingConfiguration()) {
				setHooks();
			}

		public:
			Key harvesterKey() const {
				return crypto::KeyPair::FromString(m_config.HarvesterPrivateKey).publicKey();
			}

			const auto& capturedStateHashes() const {
				return m_capturedStateHashes;
			}

			const auto& capturedSourceIdentities() const {
				return m_capturedSourceIdentities;
			}

		public:
			void enableVerifiableState() {
				auto& config = testState().state().config();
				const_cast<bool&>(config.Node.EnableCacheDatabaseStorage) = true;
				const_cast<bool&>(config.BlockChain.EnableVerifiableState) = true;
			}

			void enableDiagnosticExtension() {
				auto& config = testState().state().config();
				const_cast<std::vector<std::string>&>(config.Extensions.Names).push_back("extension.diagnostics");
			}

			void setDataDirectory(const std::string& dataDirectory) {
				auto& config = testState().state().config();
				const_cast<std::string&>(config.User.DataDirectory) = dataDirectory;
			}

		public:
			void boot() {
				ServiceLocatorTestContext::boot(m_config);
			}

		private:
			void setHooks() {
				// set up hooks
				auto& capturedStateHashes = m_capturedStateHashes;
				auto& capturedSourceIdentities = m_capturedSourceIdentities;
				testState().state().hooks().setCompletionAwareBlockRangeConsumerFactory([&](auto) {
					return [&](auto&& blockRange, auto) {
						for (const auto& block : blockRange.Range)
							capturedStateHashes.push_back(block.StateHash);

						capturedSourceIdentities.push_back(blockRange.SourceIdentity);
						return disruptor::DisruptorElementId();
					};
				});
			}

		private:
			HarvestingConfiguration m_config;
			std::vector<Hash256> m_capturedStateHashes;
			std::vector<model::NodeIdentity> m_capturedSourceIdentities;
		};

		std::shared_ptr<UnlockedAccounts> GetUnlockedAccounts(const extensions::ServiceLocator& locator) {
			return locator.service<UnlockedAccounts>(Service_Name);
		}
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(Harvesting, Post_Range_Consumers)

	// region unlocked accounts

	namespace {
		template<typename TAction>
		void RunUnlockedAccountsServiceTest(TestContext& context, TAction action) {
			// Act:
			context.boot();

			// Assert:
			EXPECT_EQ(1u, context.locator().numServices());
			EXPECT_EQ(1u, context.locator().counters().size());

			auto pUnlockedAccounts = GetUnlockedAccounts(context.locator());
			ASSERT_TRUE(!!pUnlockedAccounts);
			action(*pUnlockedAccounts);
		}
	}

	TEST(TEST_CLASS, UnlockedAccountsServiceIsRegisteredProperlyWhenAutoHarvestingIsEnabled) {
		// Arrange:
		TestContext context(test::LocalNodeFlags::Should_Auto_Harvest);
		RunUnlockedAccountsServiceTest(context, [&context](const auto& unlockedAccounts) {
			// Assert: a single account was unlocked
			EXPECT_EQ(1u, context.counter("UNLKED ACCTS"));
			EXPECT_TRUE(unlockedAccounts.view().contains(context.harvesterKey()));
		});
	}

	TEST(TEST_CLASS, UnlockedAccountsServiceIsRegisteredProperlyWhenAutoHarvestingIsDisabled) {
		// Arrange:
		TestContext context(test::LocalNodeFlags::None);
		RunUnlockedAccountsServiceTest(context, [&context](const auto& unlockedAccounts) {
			// Assert: no accounts were unlocked
			EXPECT_EQ(0u, context.counter("UNLKED ACCTS"));
			EXPECT_FALSE(unlockedAccounts.view().contains(context.harvesterKey()));
		});
	}

	TEST(TEST_CLASS, UnlockedAccountsServiceIsRegisteredProperlyWhenAutoHarvestingIsDisabledAndHarvesterPrivateKeyIsEmpty) {
		// Arrange:
		auto config = CreateHarvestingConfiguration();
		config.HarvesterPrivateKey.clear();

		TestContext context(config);
		RunUnlockedAccountsServiceTest(context, [&context](const auto&) {
			// Assert: no accounts were unlocked
			EXPECT_EQ(0u, context.counter("UNLKED ACCTS"));
		});
	}

	namespace {
		auto CreateKeyPairs(size_t numKeyPairs) {
			std::vector<crypto::KeyPair> keyPairs;
			for (auto i = 0u; i < numKeyPairs; ++i)
				keyPairs.push_back(test::GenerateKeyPair());

			return keyPairs;
		}

		void AddAccounts(
				TestContext& context,
				const std::vector<crypto::KeyPair>& keyPairs,
				const consumer<state::AccountState&>& accountStateModifier = [](const auto&) {}) {
			auto& cache = context.testState().state().cache();
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

			for (const auto& keyPair : keyPairs) {
				const auto& publicKey = keyPair.publicKey();
				accountStateCacheDelta.addAccount(publicKey, Height(100));
				accountStateModifier(accountStateCacheDelta.find(publicKey).get());
			}

			cache.commit(Height(100));
		}

		std::vector<crypto::KeyPair> AddAccountsWithImportances(TestContext& context, const std::vector<Importance>& importances) {
			auto keyPairs = CreateKeyPairs(importances.size());
			auto iter = importances.cbegin();
			AddAccounts(context, keyPairs, [iter](auto& accountState) mutable {
				accountState.ImportanceSnapshots.set(*iter, model::ImportanceHeight(100));
				++iter;
			});

			return keyPairs;
		}

		void RunUnlockedAccountsPrioritizationTest(
				DelegatePrioritizationPolicy prioritizationPolicy,
				std::initializer_list<size_t> expectedIndexes) {
			// Arrange:
			auto config = CreateHarvestingConfiguration(test::LocalNodeFlags::Should_Auto_Harvest);
			config.MaxUnlockedAccounts = 5;
			config.DelegatePrioritizationPolicy = prioritizationPolicy;

			TestContext context(config);
			auto keyPairs = AddAccountsWithImportances(context, {
				Importance(100), Importance(200), Importance(50), Importance(150), Importance(250)
			});

			RunUnlockedAccountsServiceTest(context, [&expectedIndexes, &context, &keyPairs](auto& unlockedAccounts) {
				// Act:
				std::vector<Key> publicKeys;
				for (auto& keyPair : keyPairs) {
					publicKeys.push_back(keyPair.publicKey());
					unlockedAccounts.modifier().add(std::move(keyPair));
				}

				// Assert:
				EXPECT_EQ(5u, context.counter("UNLKED ACCTS"));

				auto unlockedAccountsView = unlockedAccounts.view();
				EXPECT_TRUE(unlockedAccountsView.contains(context.harvesterKey()));
				for (auto i : expectedIndexes)
					EXPECT_TRUE(unlockedAccountsView.contains(publicKeys[i])) << "public key " << i;
			});
		}
	}

	TEST(TEST_CLASS, UnlockedAccountsServiceIsRegisteredProperlyWhenAutoHarvestingIsEnabledWithAgePrioritizationPolicy) {
		RunUnlockedAccountsPrioritizationTest(DelegatePrioritizationPolicy::Age, { 0, 1, 2, 3 });
	}

	TEST(TEST_CLASS, UnlockedAccountsServiceIsRegisteredProperlyWhenAutoHarvestingIsEnabledWithImportancePrioritizationPolicy) {
		RunUnlockedAccountsPrioritizationTest(DelegatePrioritizationPolicy::Importance, { 0, 1, 3, 4 });
	}

	namespace {
		void AddHarvestersFileEntries(const std::string& filename, const Key& nodeOwnerPublicKey, size_t numEntries) {
			UnlockedAccountsStorage storage(filename);
			for (auto i = 0u; i < numEntries; ++i) {
				auto privateKeyBuffer = test::GenerateRandomByteArray<Key>();
				auto entry = test::PrepareUnlockedTestEntry(nodeOwnerPublicKey, privateKeyBuffer);
				storage.add(entry.Key, entry.Payload, Key{ { static_cast<uint8_t>(i) } });
			}
		}
	}

	TEST(TEST_CLASS, UnlockedAccountsServiceIsLoadingUnlockedHarvestersFile) {
		// Arrange:
		test::TempDirectoryGuard directoryGuard;
		TestContext context(test::LocalNodeFlags::None);
		context.setDataDirectory(directoryGuard.name());

		auto filename = config::CatapultDataDirectory(directoryGuard.name()).rootDir().file("harvesters.dat");
		AddHarvestersFileEntries(filename, context.locator().keyPair().publicKey(), 3);

		RunUnlockedAccountsServiceTest(context, [&context](const auto& unlockedAccounts) {
			// Assert: only accounts from the file were unlocked
			EXPECT_EQ(3u, context.counter("UNLKED ACCTS"));
			EXPECT_FALSE(unlockedAccounts.view().contains(context.harvesterKey()));
		});
	}

	// endregion

	// region packet handler

	TEST(TEST_CLASS, PacketHandlerIsNotRegisteredWhenDiagnosticExtensionIsDisabled) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(0u, context.testState().state().packetHandlers().size());
	}

	TEST(TEST_CLASS, PacketHandlerIsRegisteredWhenDiagnosticExtensionIsEnabled) {
		// Arrange:
		TestContext context;
		context.enableDiagnosticExtension();

		// Act:
		context.boot();

		// Assert:
		const auto& packetHandlers = context.testState().state().packetHandlers();
		EXPECT_EQ(1u, packetHandlers.size());
		EXPECT_TRUE(packetHandlers.canProcess(ionet::PacketType::Unlocked_Accounts));
	}

	namespace {
		class DiagnosticEnabledTestContext : public TestContext {
		public:
			DiagnosticEnabledTestContext() {
				enableDiagnosticExtension();
			}
		};
	}

	ADD_HANDLERS_TRUSTED_HOSTS_TESTS(DiagnosticEnabledTestContext, ionet::PacketType::Unlocked_Accounts)

	namespace {
		auto GetPublicKeys(const std::vector<crypto::KeyPair>& keyPairs) {
			auto keys = test::Apply(true, keyPairs, [](const auto& keyPair) { return keyPair.publicKey(); });
			return std::set<Key>(keys.cbegin(), keys.cend());
		}

		void AssertPacketHandlerReturnsUnlockedAccounts(
				std::vector<crypto::KeyPair>&& keyPairs,
				const consumer<const ionet::ServerPacketHandlerContext&>& assertPacket) {
			// Arrange:
			auto config = CreateHarvestingConfiguration(test::LocalNodeFlags::None);

			TestContext context(config);
			AddAccounts(context, keyPairs);
			context.enableDiagnosticExtension();
			context.boot();

			// - add key pairs to unlocked accounts
			auto pUnlockedAccounts = GetUnlockedAccounts(context.locator());
			ASSERT_TRUE(!!pUnlockedAccounts);

			for (auto& keyPair : keyPairs)
				pUnlockedAccounts->modifier().add(std::move(keyPair));

			// Sanity:
			EXPECT_EQ(keyPairs.size(), pUnlockedAccounts->view().size());

			// Act:
			const auto& packetHandlers = context.testState().state().packetHandlers();

			// - process unlocked acconuts request
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
			pPacket->Type = ionet::PacketType::Unlocked_Accounts;
			ionet::ServerPacketHandlerContext handlerContext({}, "");
			EXPECT_TRUE(packetHandlers.process(*pPacket, handlerContext));

			assertPacket(handlerContext);
		}
	}

	TEST(TEST_CLASS, PacketHandlerReturnsEmptyPacketWhenNoUnlockedAccountsArePresent) {
		AssertPacketHandlerReturnsUnlockedAccounts(CreateKeyPairs(0), [](const auto& handlerContext) {
			// Assert: only header is present
			auto expectedPacketSize = sizeof(ionet::PacketHeader);
			test::AssertPacketHeader(handlerContext, expectedPacketSize, ionet::PacketType::Unlocked_Accounts);
		});
	}

	TEST(TEST_CLASS, PacketHandlerReturnsUnlockedAccounts) {
		// Arrange:
		auto keyPairs = CreateKeyPairs(3);
		auto expectedPublicKeys = GetPublicKeys(keyPairs);
		AssertPacketHandlerReturnsUnlockedAccounts(std::move(keyPairs), [&expectedPublicKeys](const auto& handlerContext) {
			// Assert: header is correct and contains the expected number of keys
			auto expectedPacketSize = sizeof(ionet::PacketHeader) + 3 * Key::Size;
			test::AssertPacketHeader(handlerContext, expectedPacketSize, ionet::PacketType::Unlocked_Accounts);

			const auto* pUnlockedPublicKeys = reinterpret_cast<const Key*>(test::GetSingleBufferData(handlerContext));
			auto unlockedPublicKeys = std::set<Key>(pUnlockedPublicKeys, pUnlockedPublicKeys + 3);

			EXPECT_EQ(expectedPublicKeys, unlockedPublicKeys);
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
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);
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
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.ImportanceGrouping = Importance_Grouping;
			config.MinHarvesterBalance = Account_Balance;
			config.MaxHarvesterBalance = Amount(std::numeric_limits<Amount::ValueType>::max());
			auto cache = test::CreateEmptyCatapultCache(config, cacheConfig);
			auto delta = cache.createDelta();

			// - add an account
			auto& accountStateCache = delta.sub<cache::AccountStateCache>();
			accountStateCache.addAccount(publicKey, Height(100));
			auto& accountState = accountStateCache.find(publicKey).get();
			accountState.ImportanceSnapshots.set(Importance(123), importanceHeight);
			accountState.Balances.credit(Harvesting_Mosaic_Id, balance);

			// - add a block statistic
			auto& blockStatisticCache = delta.sub<cache::BlockStatisticCache>();
			blockStatisticCache.insert(state::BlockStatistic(Height(1)));

			// - commit changes
			delta.calculateStateHash(Height(1));
			cache.commit(height);
			return cache;
		}

		auto CreateCacheWithAccount(Height height, const Key& publicKey, Amount balance, model::ImportanceHeight importanceHeight) {
			return CreateCacheWithAccount(cache::CacheConfiguration(), height, publicKey, balance, importanceHeight);
		}
	}

	TEST(TEST_CLASS, HarvestingTaskIsScheduled) {
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
		TestContext context(CreateCacheWithAccount(height, keyPair.publicKey(), Account_Balance - Amount(1), importanceHeight));

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
			test::TempDirectoryGuard dbDirGuard;
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

				// - source public key is zero indicating harvester
				ASSERT_EQ(1u, context.capturedSourceIdentities().size());
				EXPECT_EQ(Key(), context.capturedSourceIdentities()[0].PublicKey);
				EXPECT_EQ("", context.capturedSourceIdentities()[0].Host);

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

	TEST(TEST_CLASS, HarvestingTaskGeneratesNonzeroStateHashWhenVerifiableStateIsEnabled) {
		// Act:
		Hash256 harvestedStateHash;
		RunHarvestingStateHashTest(true, harvestedStateHash);

		// Assert:
		EXPECT_NE(Hash256(), harvestedStateHash);
	}

	// endregion
}}
