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

#include "filechain/src/FileBlockChainStorage.h"
#include "plugins/services/hashcache/src/cache/HashCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/io/BlockStorageCache.h"
#include "filechain/tests/test/FilechainTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/BlockStateHash.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/RealTransactionFactory.h"
#include "tests/test/nemesis/NemesisTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"
#include <random>

namespace catapult { namespace filechain {

#define TEST_CLASS FileBlockChainStorageTests

	namespace {
		// region TestContext

		model::BlockChainConfiguration CreateBlockChainConfiguration(uint32_t maxDifficultyBlocks, const std::string& dataDirectory) {
			return test::CreateFileChainLocalNodeConfiguration(maxDifficultyBlocks, dataDirectory).BlockChain;
		}

		class TestContext {
		public:
			explicit TestContext(const std::shared_ptr<plugins::PluginManager>& pPluginManager, const std::string& dataDirectory)
					: m_pPluginManager(pPluginManager)
					, m_localNodeState(m_pPluginManager->config(), dataDirectory, m_pPluginManager->createCache())
					, m_pBlockChainStorage(CreateFileBlockChainStorage())
			{}

			explicit TestContext(const model::BlockChainConfiguration& config, const std::string& dataDirectory)
					: TestContext(test::CreatePluginManager(config), dataDirectory)
			{}

			explicit TestContext(uint32_t maxDifficultyBlocks = 0, const std::string& dataDirectory = "")
					: TestContext(CreateBlockChainConfiguration(maxDifficultyBlocks, dataDirectory), dataDirectory)
			{}

		public:
			auto storageModifier() {
				return m_localNodeState.ref().Storage.modifier();
			}

			auto storageView() {
				return m_localNodeState.ref().Storage.view();
			}

			auto cacheView() const {
				return m_localNodeState.cref().Cache.createView();
			}

			auto score() const {
				return m_localNodeState.cref().Score.get();
			}

			auto lastRecalculationHeight() const {
				return m_localNodeState.cref().State.LastRecalculationHeight;
			}

		public:
			void load() {
				m_pBlockChainStorage->loadFromStorage(m_localNodeState.ref(), *m_pPluginManager);
			}

			void save() const {
				m_pBlockChainStorage->saveToStorage(m_localNodeState.cref());
			}

		public:
			void enableCacheDatabaseStorage(bool enable) {
				const_cast<bool&>(m_pPluginManager->storageConfig().PreferCacheDatabase) = enable;
				const_cast<bool&>(m_localNodeState.ref().Config.Node.ShouldUseCacheDatabaseStorage) = enable;
			}

		private:
			std::shared_ptr<plugins::PluginManager> m_pPluginManager;
			test::LocalNodeTestState m_localNodeState;

			std::unique_ptr<extensions::BlockChainStorage> m_pBlockChainStorage;
		};

		// endregion
	}

	// region basic nemesis loading

	TEST(TEST_CLASS, ProperAccountStateAfterLoadingNemesisBlock) {
		// Arrange:
		TestContext context;

		// Act:
		context.load();

		// Assert:
		const auto& view = context.cacheView();
		EXPECT_EQ(Height(1), view.height());
		test::AssertNemesisAccountState(view);
	}

	TEST(TEST_CLASS, ProperMosaicStateAfterLoadingNemesisBlock) {
		// Arrange:
		TestContext context;

		// Act:
		context.load();

		// Assert:
		const auto& view = context.cacheView();
		test::AssertNemesisNamespaceState(view);
		test::AssertNemesisMosaicState(view);
	}

	TEST(TEST_CLASS, ProperChainScoreAfterLoadingNemesisBlock) {
		// Arrange:
		TestContext context;

		// Act:
		context.load();

		// Assert:
		EXPECT_EQ(model::ChainScore(), context.score());
	}

	TEST(TEST_CLASS, ProperRecalculationHeightAfterLoadingNemesisBlock) {
		// Arrange:
		TestContext context;

		// Act:
		context.load();

		// Assert:
		EXPECT_EQ(model::ImportanceHeight(1), context.lastRecalculationHeight());
	}

	// endregion

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
		constexpr auto Num_Nemesis_Accounts = CountOf(test::Mijin_Test_Private_Keys);
		constexpr auto Num_Nemesis_Namespaces = 1;
		constexpr auto Num_Nemesis_Mosaics = 1;
		constexpr auto Num_Recipient_Accounts = 10 * Num_Nemesis_Accounts;
		constexpr Amount Nemesis_Recipient_Amount(409'090'909'000'000);
	}

	// region PrepareRandomBlocks

	namespace {
		struct RandomChainAttributes {
			std::vector<Address> Recipients;
			std::vector<size_t> TransactionCounts;
		};

		std::vector<Address> GenerateRandomAddresses(size_t count) {
			std::vector<Address> addresses;
			for (auto i = 0u; i < count; ++i)
				addresses.push_back(test::GenerateRandomAddress());

			return addresses;
		}

		RandomChainAttributes PrepareRandomBlocks(
				io::BlockStorageModifier&& storage,
				std::vector<Amount>& amountsSpent,
				std::vector<Amount>& amountsCollected,
				const utils::TimeSpan& timeSpacing) {
			RandomChainAttributes attributes;
			amountsSpent.resize(Num_Nemesis_Accounts);
			amountsCollected.resize(Num_Recipient_Accounts);
			attributes.Recipients = GenerateRandomAddresses(Num_Recipient_Accounts);

			// Generate block per every recipient, each with random number of transactions.
			auto recipientIndex = 0u;
			auto height = 2u;
			std::mt19937_64 rnd;
			auto nemesisKeyPairs = test::GetNemesisKeyPairs();
			for (const auto& recipientAddress : attributes.Recipients) {
				auto blockWithAttributes = test::CreateBlock(nemesisKeyPairs, recipientAddress, rnd, height, timeSpacing);

				attributes.TransactionCounts.push_back(blockWithAttributes.SenderIds.size());
				for (auto i = 0u; i < blockWithAttributes.SenderIds.size(); ++i) {
					auto senderIndex = blockWithAttributes.SenderIds[i];
					auto amount = blockWithAttributes.Amounts[i];
					amountsSpent[senderIndex] = amountsSpent[senderIndex] + amount;
					amountsCollected[recipientIndex] = amountsCollected[recipientIndex] + amount;
				}

				storage.saveBlock(test::BlockToBlockElement(*blockWithAttributes.pBlock));
				++height;
				++recipientIndex;
			}

			return attributes;
		}

		RandomChainAttributes PrepareRandomBlocks(io::BlockStorageModifier&& storage, const utils::TimeSpan& timeSpacing) {
			std::vector<Amount> amountsSpent;
			std::vector<Amount> amountsCollected;
			return PrepareRandomBlocks(std::move(storage), amountsSpent, amountsCollected, timeSpacing);
		}

		void AssertNemesisAccount(const cache::AccountStateCacheView& view) {
			auto nemesisKeyPair = crypto::KeyPair::FromString(test::Mijin_Test_Nemesis_Private_Key);
			auto address = model::PublicKeyToAddress(nemesisKeyPair.publicKey(), Network_Identifier);

			auto nemesisAccountStateIter = view.find(address);
			const auto& nemesisAccountState = nemesisAccountStateIter.get();
			EXPECT_EQ(Height(1), nemesisAccountState.AddressHeight);
			EXPECT_EQ(Height(1), nemesisAccountState.PublicKeyHeight);
			EXPECT_EQ(0u, nemesisAccountState.Balances.size());
		}

		void AssertNemesisRecipient(const cache::AccountStateCacheView& view, const Address& address, Amount amountSpent) {
			auto message = model::AddressToString(address);
			auto accountStateIter = view.find(address);
			const auto& accountState = accountStateIter.get();

			EXPECT_EQ(Height(1), accountState.AddressHeight) << message;

			if (Amount(0) != amountSpent)
				EXPECT_LT(Height(0), accountState.PublicKeyHeight) << message;

			EXPECT_EQ(Nemesis_Recipient_Amount - amountSpent, accountState.Balances.get(Xem_Id)) << message;
		}

		void AssertSecondaryRecipient(const cache::AccountStateCacheView& view, const Address& address, size_t i, Amount amountReceived) {
			auto message = model::AddressToString(address) + " " + std::to_string(i);
			auto accountStateIter = view.find(address);
			const auto& accountState = accountStateIter.get();

			EXPECT_EQ(Height(i + 2), accountState.AddressHeight) << message;
			EXPECT_EQ(Height(0), accountState.PublicKeyHeight) << message;
			EXPECT_EQ(amountReceived, accountState.Balances.get(Xem_Id)) << message;
		}
	}

	// endregion

	// region multi block loading - ProperAccountCacheState

	namespace {
		void SetNemesisStateHash(TestContext& context, const config::LocalNodeConfiguration& config) {
			auto pNemesisBlockElement = context.storageView().loadBlockElement(Height(1));

			auto pModifiedBlock = test::CopyBlock(pNemesisBlockElement->Block);
			auto modifiedNemesisBlockElement = test::BlockToBlockElement(*pModifiedBlock);
			modifiedNemesisBlockElement.GenerationHash = pNemesisBlockElement->GenerationHash;

			auto nemesisStateHash = test::CalculateNemesisStateHash(modifiedNemesisBlockElement, config);
			auto modifier = context.storageModifier();
			modifier.dropBlocksAfter(Height(0));

			// add state hash to nemesis block and resign it
			pModifiedBlock->StateHash = nemesisStateHash;
			test::SignBlock(crypto::KeyPair::FromString(test::Mijin_Test_Nemesis_Private_Key), *pModifiedBlock);

			modifier.saveBlock(modifiedNemesisBlockElement);
		}

		void AssertProperAccountCacheStateAfterLoadingMultipleBlocksUsingContext(
				TestContext& context,
				const utils::TimeSpan& timeSpacing) {
			std::vector<Amount> amountsSpent; // amounts spent by nemesis accounts to send to other newAccounts
			std::vector<Amount> amountsCollected;
			auto newAccounts = PrepareRandomBlocks(context.storageModifier(), amountsSpent, amountsCollected, timeSpacing).Recipients;

			// Act:
			context.load();

			// Assert:
			auto i = 0u;
			auto cacheView = context.cacheView();
			const auto& accountStateCacheView = cacheView.sub<cache::AccountStateCache>();

			// - check nemesis
			AssertNemesisAccount(accountStateCacheView);

			// - check nemesis recipients
			for (const auto* pRecipientPrivateKeyString : test::Mijin_Test_Private_Keys) {
				auto recipient = crypto::KeyPair::FromString(pRecipientPrivateKeyString);
				auto address = model::PublicKeyToAddress(recipient.publicKey(), Network_Identifier);
				AssertNemesisRecipient(accountStateCacheView, address, amountsSpent[i]);
				++i;
			}

			// - check secondary recipients
			i = 0;
			for (const auto& address : newAccounts) {
				AssertSecondaryRecipient(accountStateCacheView, address, i, amountsCollected[i]);
				++i;
			}
		}

		void FixNemesisBlockStateHash(TestContext& context) {
			// there are two plugin managers:
			// - one created in test context
			// - second created one via test::CalculateNemesisStateHash
			// separate config is needed so that rdb will be created in separate dir
			test::TempDirectoryGuard tempDataDirectory("../temp2.dir");
			auto nemesisConfig = test::CreateStateHashEnabledLocalNodeConfiguration(tempDataDirectory.name());
			SetNemesisStateHash(context, nemesisConfig);
		}

		struct AccountStateTraits_StateHashEnabled {
		public:
			static void AssertProperAccountCacheStateAfterLoadingMultipleBlocks(const utils::TimeSpan& timeSpacing) {
				// Arrange:
				test::TempDirectoryGuard tempDataDirectory;
				auto config = test::CreateStateHashEnabledLocalNodeConfiguration(tempDataDirectory.name());
				TestContext context(test::CreatePluginManager(config), tempDataDirectory.name());
				FixNemesisBlockStateHash(context);

				// Assert:
				AssertProperAccountCacheStateAfterLoadingMultipleBlocksUsingContext(context, timeSpacing);
			}
		};

		struct AccountStateTraits {
		public:
			static void AssertProperAccountCacheStateAfterLoadingMultipleBlocks(const utils::TimeSpan& timeSpacing) {
				// Arrange:
				TestContext context;

				// Assert:
				AssertProperAccountCacheStateAfterLoadingMultipleBlocksUsingContext(context, timeSpacing);
			}
		};
	}

#define ACCOUNT_CACHE_STATE_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_StateHashEnabled) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountStateTraits_StateHashEnabled>(); } \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountStateTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ACCOUNT_CACHE_STATE_TRAITS_BASED_TEST(ProperAccountCacheStateAfterLoadingMultipleBlocks_AllBlocksContributeToTransientState) {
		// Assert:
		TTraits::AssertProperAccountCacheStateAfterLoadingMultipleBlocks(utils::TimeSpan::FromSeconds(1));
	}

	ACCOUNT_CACHE_STATE_TRAITS_BASED_TEST(ProperAccountCacheStateAfterLoadingMultipleBlocks_SomeBlocksContributeToTransientState) {
		// Assert: account state is permanent and should not be short-circuited
		TTraits::AssertProperAccountCacheStateAfterLoadingMultipleBlocks(utils::TimeSpan::FromMinutes(1));
	}

	// endregion

	// region multi block loading - ProperCacheHeight

	namespace {
		void AssertProperCacheHeightAfterLoadingMultipleBlocksUsingContext(TestContext& context, const utils::TimeSpan& timeSpacing) {
			// Arrange:
			PrepareRandomBlocks(context.storageModifier(), timeSpacing);

			// Act:
			context.load();

			// Assert:
			auto cacheView = context.cacheView();
			EXPECT_EQ(Height(Num_Recipient_Accounts + 1), cacheView.height());
		}

		struct CacheHeightTraits_StateHashEnabled {
		public:
			static void AssertProperCacheHeightAfterLoadingMultipleBlocks(const utils::TimeSpan& timeSpacing) {
				// Arrange:
				test::TempDirectoryGuard tempDataDirectory;
				auto config = test::CreateStateHashEnabledLocalNodeConfiguration(tempDataDirectory.name());
				TestContext context(test::CreatePluginManager(config), tempDataDirectory.name());
				FixNemesisBlockStateHash(context);

				// Assert:
				AssertProperCacheHeightAfterLoadingMultipleBlocksUsingContext(context, timeSpacing);
			}
		};

		struct CacheHeightTraits {
		public:
			static void AssertProperCacheHeightAfterLoadingMultipleBlocks(const utils::TimeSpan& timeSpacing) {
				// Arrange:
				TestContext context;

				// Assert:
				AssertProperCacheHeightAfterLoadingMultipleBlocksUsingContext(context, timeSpacing);
			}
		};
	}

#define CACHE_HEIGHT_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_StateHashEnabled) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CacheHeightTraits_StateHashEnabled>(); } \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CacheHeightTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CACHE_HEIGHT_TRAITS_BASED_TEST(ProperCacheHeightAfterLoadingMultipleBlocks_AllBlocksContributeToTransientState) {
		// Assert:
		TTraits::AssertProperCacheHeightAfterLoadingMultipleBlocks(utils::TimeSpan::FromSeconds(1));
	}

	CACHE_HEIGHT_TRAITS_BASED_TEST(ProperCacheHeightAfterLoadingMultipleBlocks_SomeBlocksContributeToTransientState) {
		// Assert: cache height is permanent and should not be short-circuited
		TTraits::AssertProperCacheHeightAfterLoadingMultipleBlocks(utils::TimeSpan::FromMinutes(1));
	}

	// endregion

	// region multi block loading - ProperChainScore

	namespace {
		void AssertProperChainScoreAfterLoadingMultipleBlocks(const utils::TimeSpan& timeSpacing) {
			// Arrange:
			TestContext context;
			PrepareRandomBlocks(context.storageModifier(), timeSpacing);

			// Act:
			context.load();

			// Assert:
			// note that there are Num_Recipient_Accounts blocks (one per recipient)
			// - each block has a difficulty of base + height
			// - all blocks except for the first one have a time difference of 1s (the first one has a difference of 2s)
			auto result = context.score();
			uint64_t expectedDifficulty =
					Difficulty().unwrap() * Num_Recipient_Accounts // sum base difficulties
					+ (Num_Recipient_Accounts + 1) * (Num_Recipient_Accounts + 2) / 2 // sum difficulty deltas (1..N+1)
					- 1 // adjust for range (2..N+1) - first 'recipient' block has height 2
					- (Num_Recipient_Accounts + 1) * timeSpacing.seconds(); // subtract time differences
			EXPECT_EQ(model::ChainScore(expectedDifficulty), result);
		}
	}

	TEST(TEST_CLASS, ProperChainScoreAfterLoadingMultipleBlocks_AllBlocksContributeToTransientState) {
		// Assert:
		AssertProperChainScoreAfterLoadingMultipleBlocks(utils::TimeSpan::FromSeconds(1));
	}

	TEST(TEST_CLASS, ProperChainScoreAfterLoadingMultipleBlocks_SomeBlocksContributeToTransientState) {
		// Assert: chain score is permanent and should not be short-circuited
		AssertProperChainScoreAfterLoadingMultipleBlocks(utils::TimeSpan::FromMinutes(1));
	}

	// endregion

	// region multi block loading - ProperTransientCacheState

	namespace {
		template<typename T>
		T Sum(const std::vector<T>& vec, size_t startIndex, size_t endIndex) {
			T sum = 0;
			for (auto i = startIndex; i <= endIndex; ++i)
				sum += vec[i];

			return sum;
		}

		void AssertBlockDifficultyCacheRange(
				const cache::BlockDifficultyCacheView& view,
				Height expectedStartHeight,
				Height expectedEndHeight) {
			auto pIterableView = view.tryMakeIterableView();
			ASSERT_TRUE(!!pIterableView);
			EXPECT_EQ(expectedStartHeight, pIterableView->begin()->BlockHeight);
			EXPECT_EQ(expectedEndHeight, std::prev(pIterableView->end())->BlockHeight);
		}
	}

	TEST(TEST_CLASS, ProperTransientCacheStateAfterLoadingMultipleBlocks_AllBlocksContributeToTransientState) {
		// Arrange:
		// - note that even though the config is zeroed, MaxTransientStateCacheDuration is 1hr because of the
		//   min RollbackVariabilityBufferDuration
		// - 1s block spacing will sum to much less than 1hr, so state from all blocks should be cached
		TestContext context;
		auto transactionCounts = PrepareRandomBlocks(context.storageModifier(), utils::TimeSpan::FromSeconds(1)).TransactionCounts;
		auto numTotalTransferTransactions = Sum(transactionCounts, 0, transactionCounts.size() - 1);

		// Act:
		context.load();

		// Assert: all hashes and difficulties were cached
		// - adjust comparisons for the nemesis block, which has
		//   1) Num_Nemesis_Namespaces register namespace transactions
		//   2) for each mosaic one mosaic definition transaction and one mosaic supply change transaction
		//   3) Num_Nemesis_Accounts transfer transactions
		auto cacheView = context.cacheView();
		EXPECT_EQ(
				numTotalTransferTransactions + Num_Nemesis_Accounts + Num_Nemesis_Namespaces + 2 * Num_Nemesis_Mosaics,
				cacheView.sub<cache::HashCache>().size());

		const auto& blockDifficultyCache = cacheView.sub<cache::BlockDifficultyCache>();
		EXPECT_EQ(transactionCounts.size() + 1, blockDifficultyCache.size());
		AssertBlockDifficultyCacheRange(blockDifficultyCache, Height(1), Height(1 + transactionCounts.size()));
	}

	namespace {
		void AssertProperTransientCacheStateAfterLoadingMultipleBlocksWithInflection(
				uint32_t maxDifficultyBlocks,
				size_t numExpectedSignificantBlocks) {
			// Arrange:
			// - note that even though the config is zeroed, MaxTransientStateCacheDuration is 1hr because of the
			//   min RollbackVariabilityBufferDuration
			// - 1m block spacing will sum to greater than 1hr, so state from some blocks should not be cached
			TestContext context(maxDifficultyBlocks);
			auto transactionCounts = PrepareRandomBlocks(context.storageModifier(), utils::TimeSpan::FromMinutes(1)).TransactionCounts;

			// Act:
			context.load();

			// Sanity: numExpectedSignificantBlocks should be a subset of all blocks
			ASSERT_LT(numExpectedSignificantBlocks, transactionCounts.size());

			auto startAllObserversIndex = transactionCounts.size() - numExpectedSignificantBlocks;
			auto numTotalTransactions = Sum(transactionCounts, startAllObserversIndex, transactionCounts.size() - 1);

			// Assert: older hashes and difficulties were not cached
			//         (note that transactionCounts indexes 0..N correspond to heights 2..N+2)
			auto cacheView = context.cacheView();
			EXPECT_EQ(numTotalTransactions, cacheView.sub<cache::HashCache>().size());

			const auto& blockDifficultyCache = cacheView.sub<cache::BlockDifficultyCache>();
			EXPECT_EQ(numExpectedSignificantBlocks, blockDifficultyCache.size());
			AssertBlockDifficultyCacheRange(
					blockDifficultyCache,
					Height(2 + startAllObserversIndex),
					Height(1 + transactionCounts.size()));
		}
	}

	TEST(TEST_CLASS, ProperTransientCacheStateAfterLoadingMultipleBlocks_SomeBlocksContributeToTransientState_TimeDominant) {
		// Assert: state from blocks at times [T - 60, T] should be cached
		AssertProperTransientCacheStateAfterLoadingMultipleBlocksWithInflection(60, 61);
	}

	TEST(TEST_CLASS, ProperTransientCacheStateAfterLoadingMultipleBlocks_SomeBlocksContributeToTransientState_HeightDominant) {
		// Assert: state from the last 75 blocks should be cached
		AssertProperTransientCacheStateAfterLoadingMultipleBlocksWithInflection(75, 75);
	}

	// endregion

	// region saveToStorage

	namespace {
		void AssertCanSaveAndReloadCacheStateToAndFromDisk(bool useCacheDatabaseStorage) {
			// Arrange:
			test::TempDirectoryGuard tempDataDirectory;
			std::vector<Amount> amountsSpent;
			std::vector<Amount> amountsCollected;
			std::vector<Address> newAccounts;

			uint32_t maxDifficultyBlocks = Num_Recipient_Accounts / 4;
			auto storageChainHeight = Height(Num_Recipient_Accounts + 1);
			{
				// - generate random state
				auto timeSpacing = utils::TimeSpan::FromMinutes(2);
				TestContext context(maxDifficultyBlocks, tempDataDirectory.name());
				context.enableCacheDatabaseStorage(useCacheDatabaseStorage);
				newAccounts = PrepareRandomBlocks(context.storageModifier(), amountsSpent, amountsCollected, timeSpacing).Recipients;
				context.load();

				// Act: save to disk
				context.save();
			}

			// Act: reload the state from the saved cache state
			TestContext context(maxDifficultyBlocks, tempDataDirectory.name());
			context.enableCacheDatabaseStorage(useCacheDatabaseStorage);
			context.load();

			// Assert: check the heights (notice that storage is empty because it was not reseeded in the second test context)
			EXPECT_EQ(storageChainHeight, context.cacheView().height());
			EXPECT_EQ(Height(1), context.storageView().chainHeight());

			// - spot check the new accounts by checking secondary recipients
			auto cacheView = context.cacheView();
			const auto& accountStateCacheView = cacheView.sub<cache::AccountStateCache>();
			auto i = 0u;
			for (const auto& address : newAccounts) {
				AssertSecondaryRecipient(accountStateCacheView, address, i, amountsCollected[i]);
				++i;
			}

			// - spot check the block difficulty cache
			const auto& blockDifficultyCache = cacheView.sub<cache::BlockDifficultyCache>();
			EXPECT_EQ(maxDifficultyBlocks, blockDifficultyCache.size());
			AssertBlockDifficultyCacheRange(
					blockDifficultyCache,
					storageChainHeight - Height(maxDifficultyBlocks) + Height(1),
					storageChainHeight);
		}
	}

	TEST(TEST_CLASS, CanSaveAndReloadCacheStateToAndFromDisk) {
		// Assert:
		AssertCanSaveAndReloadCacheStateToAndFromDisk(false);
	}

	TEST(TEST_CLASS, CanSaveAndReloadCacheStateToAndFromDiskWhenCacheDatabaseStorageIsEnabled) {
		// Assert:
		AssertCanSaveAndReloadCacheStateToAndFromDisk(true);
	}

	namespace {
		template<typename TAction>
		void RunReloadWithInconsistentCacheAndStorageHeightTest(bool useCacheDatabaseStorage, TAction action) {
			// Arrange:
			test::TempDirectoryGuard tempDataDirectory;
			std::vector<Amount> amountsSpent;
			std::vector<Amount> amountsCollected;
			std::vector<Address> newAccounts;

			uint32_t maxDifficultyBlocks = Num_Recipient_Accounts / 4;
			auto savedCacheStateHeight = Height(Num_Recipient_Accounts / 2);
			auto storageChainHeight = Height(Num_Recipient_Accounts + 1);

			// - force a prune at the last block and create a context for (re)loading
			auto config = CreateBlockChainConfiguration(maxDifficultyBlocks, tempDataDirectory.name());
			config.BlockPruneInterval = static_cast<uint32_t>(storageChainHeight.unwrap());
			TestContext context(config, tempDataDirectory.name());
			context.enableCacheDatabaseStorage(useCacheDatabaseStorage);
			{
				// - generate random state
				auto timeSpacing = utils::TimeSpan::FromMinutes(2);
				TestContext seedContext(config, tempDataDirectory.name());
				seedContext.enableCacheDatabaseStorage(useCacheDatabaseStorage);
				newAccounts = PrepareRandomBlocks(seedContext.storageModifier(), amountsSpent, amountsCollected, timeSpacing).Recipients;

				// - drop half the blocks
				seedContext.storageModifier().dropBlocksAfter(savedCacheStateHeight);
				seedContext.load();

				// Sanity:
				EXPECT_EQ(savedCacheStateHeight, seedContext.cacheView().height());

				// Act: save to disk
				seedContext.save();

				// - reset the storage height and copy all blocks into the second context (used to reload the state)
				seedContext.storageModifier().dropBlocksAfter(storageChainHeight);
				for (auto height = Height(2); height <= storageChainHeight; height = height + Height(1))
					context.storageModifier().saveBlock(*seedContext.storageView().loadBlockElement(height));
			}

			action(context, maxDifficultyBlocks, amountsCollected, newAccounts);
		}
	}

	TEST(TEST_CLASS, CanSaveAndReloadPartialCacheStateToAndFromDiskAndLoadRemainingStateFromAdditionalStorageBlocks) {
		// Arrange:
		RunReloadWithInconsistentCacheAndStorageHeightTest(false, [](
				auto& context,
				auto maxDifficultyBlocks,
				const auto& amountsCollected,
				const auto& newAccounts) {
			auto storageChainHeight = Height(Num_Recipient_Accounts + 1);

			// Act: reload the state from the saved cache state and storage
			context.load();

			// Assert: check the heights
			EXPECT_EQ(storageChainHeight, context.cacheView().height());
			EXPECT_EQ(storageChainHeight, context.storageView().chainHeight());

			// - spot check the new accounts by checking secondary recipients
			auto cacheView = context.cacheView();
			const auto& accountStateCacheView = cacheView.template sub<cache::AccountStateCache>();
			auto i = 0u;
			for (const auto& address : newAccounts) {
				AssertSecondaryRecipient(accountStateCacheView, address, i, amountsCollected[i]);
				++i;
			}

			// - spot check the block difficulty cache (notice that pruning leaves an extra entry in the cache)
			const auto& blockDifficultyCache = cacheView.template sub<cache::BlockDifficultyCache>();
			EXPECT_EQ(maxDifficultyBlocks + 1, blockDifficultyCache.size());
			AssertBlockDifficultyCacheRange(blockDifficultyCache, storageChainHeight - Height(maxDifficultyBlocks), storageChainHeight);
		});
	}

	TEST(TEST_CLASS, CannotLoadRemainingStateFromAdditionalStorageBlocksWhenCacheDatabaseStorageIsEnabled) {
		// Arrange:
		RunReloadWithInconsistentCacheAndStorageHeightTest(true, [](auto& context, auto, const auto&, const auto&) {
			// Act + Assert: reload the state from the saved cache state and storage (it should fail because of inconsistent heights)
			EXPECT_THROW(context.load(), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, CannotLoadCorruptedCacheStateFromDisk) {
		// Arrange:
		test::TempDirectoryGuard tempDataDirectory;
		{
			// - generate random state
			auto timeSpacing = utils::TimeSpan::FromMinutes(1);
			TestContext context(0, tempDataDirectory.name());
			PrepareRandomBlocks(context.storageModifier(), timeSpacing);
			context.load();

			// - save to disk
			context.save();

			// - delete a cache state file
			auto cacheStateFilename = boost::filesystem::path(tempDataDirectory.name()) / "state" / "BlockDifficultyCache.dat";
			remove(cacheStateFilename.generic_string().c_str());
		}

		// Act + Assert: reload the state from the saved cache state (the reload should fail due to incomplete saved cache state)
		TestContext context(0, tempDataDirectory.name());
		EXPECT_THROW(context.load(), catapult_runtime_error);
	}

	// endregion
}}
