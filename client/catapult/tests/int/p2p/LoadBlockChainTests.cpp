#include "catapult/local/p2p/LocalNode.h"
#include "plugins/services/hashcache/src/cache/HashCache.h"
#include "catapult/cache/AccountStateCache.h"
#include "catapult/cache/BlockDifficultyCache.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/io/FileBasedStorage.h"
#include "catapult/model/Address.h"
#include "catapult/model/ChainScore.h"
#include "catapult/state/AccountState.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/test/int/Configuration.h"
#include "tests/test/int/NemesisTestUtils.h"
#include "tests/test/local/EntityFactory.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/MijinConstants.h"
#include "tests/TestHarness.h"
#include <random>

namespace catapult { namespace local { namespace p2p {
	namespace {
		// region TestContext

		class TestContext {
		public:
			explicit TestContext(uint32_t maxDifficultyBlocks = 0)
					: m_maxDifficultyBlocks(maxDifficultyBlocks)
					, m_serverKeyPair(test::LoadServerKeyPair()) {
				test::PrepareStorage(m_tempDir.name());

				reboot();
			}

		public:
			const auto& cache() {
				return m_pLocalNode->cache();
			}

			auto score() {
				return m_pLocalNode->score();
			}

			auto storage() {
				return std::make_unique<io::FileBasedStorage>(m_tempDir.name());
			}

		public:
			void reboot() {
				auto config = test::LoadLocalNodeConfigurationWithNemesisPluginExtensions(m_tempDir.name());
				if (m_maxDifficultyBlocks > 0)
					const_cast<model::BlockChainConfiguration&>(config.BlockChain).MaxDifficultyBlocks = m_maxDifficultyBlocks;

				m_pLocalNode = CreateLocalNode(
						m_serverKeyPair,
						std::move(config),
						std::make_unique<thread::MultiServicePool>(
								thread::MultiServicePool::DefaultPoolConcurrency(),
								"LocalNodeTests-p2p"));
			}

		private:
			uint32_t m_maxDifficultyBlocks;
			crypto::KeyPair m_serverKeyPair;
			test::TempDirectoryGuard m_tempDir;
			std::unique_ptr<BootedLocalNode> m_pLocalNode;
		};

		// endregion
	}

	// region basic nemesis loading

	TEST(LoadBlockChainTests, ProperAccountStateAfterLoadingNemesisBlock) {
		// Act:
		TestContext context;

		// Assert:
		const auto& view = context.cache().createView();
		EXPECT_EQ(Height(1), view.height());
		test::AssertNemesisAccountState(view);
	}

	TEST(LoadBlockChainTests, ProperMosaicStateAfterLoadingNemesisBlock) {
		// Act:
		TestContext context;

		// Assert:
		const auto& view = context.cache().createView();
		test::AssertNemesisNamespaceState(view);
		test::AssertNemesisMosaicState(view);
	}

	TEST(LoadBlockChainTests, ProperChainScoreAfterLoadingNemesisBlock) {
		// Act:
		TestContext context;

		// Assert:
		EXPECT_EQ(model::ChainScore(), context.score());
	}

	// endregion

	// region multi block loading

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
		constexpr auto Num_Nemesis_Accounts = CountOf(test::Mijin_Test_Private_Keys);
		constexpr auto Num_Nemesis_Namespaces = 1;
		constexpr auto Num_Nemesis_Mosaics = 1;
		constexpr auto Num_Recipient_Accounts = 10 * Num_Nemesis_Accounts;
		constexpr Amount Nemesis_Recipient_Amount(818'181'818'000'000);
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

		std::vector<crypto::KeyPair> GetNemesisKeyPairs() {
			std::vector<crypto::KeyPair> nemesisKeyPairs;
			for (const auto* pRecipientPrivateKeyString : test::Mijin_Test_Private_Keys)
				nemesisKeyPairs.push_back(crypto::KeyPair::FromString(pRecipientPrivateKeyString));

			return nemesisKeyPairs;
		}

		RandomChainAttributes PrepareRandomBlocks(
				io::LightBlockStorage&& storage,
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
			auto nemesisKeyPairs = GetNemesisKeyPairs();
			for (const auto& recipientAddress : attributes.Recipients) {
				std::uniform_int_distribution<size_t> numTransactionsDistribution(5, 20);
				auto numTransactions = numTransactionsDistribution(rnd);
				attributes.TransactionCounts.push_back(numTransactions);

				test::ConstTransactions transactions;
				std::uniform_int_distribution<size_t> accountIndexDistribution(0, Num_Nemesis_Accounts - 1);
				for (auto i = 0u; i < numTransactions; ++i) {
					auto senderIndex = accountIndexDistribution(rnd);
					const auto& sender = nemesisKeyPairs[senderIndex];

					std::uniform_int_distribution<Amount::ValueType> amountDistribution(1000, 10 * 1000);
					Amount amount(amountDistribution(rnd) * 1'000'000u);
					auto pTransaction = test::CreateUnsignedTransferTransaction(sender.publicKey(), recipientAddress, amount);
					pTransaction->Fee = Amount(0);
					transactions.push_back(std::move(pTransaction));
					amountsSpent[senderIndex] = amountsSpent[senderIndex] + amount;
					amountsCollected[recipientIndex] = amountsCollected[recipientIndex] + amount;
				}

				auto harvesterIndex = accountIndexDistribution(rnd);
				auto pBlock = test::GenerateBlockWithTransactions(nemesisKeyPairs[harvesterIndex], transactions);
				pBlock->Height = Height(height);
				pBlock->Difficulty = Difficulty(Difficulty().unwrap() + height);
				pBlock->Timestamp = Timestamp(height * timeSpacing.millis());
				storage.saveBlock(test::BlockToBlockElement(*pBlock));
				++height;
				++recipientIndex;
			}

			return attributes;
		}

		RandomChainAttributes PrepareRandomBlocks(io::LightBlockStorage&& storage, const utils::TimeSpan& timeSpacing) {
			std::vector<Amount> amountsSpent;
			std::vector<Amount> amountsCollected;
			return PrepareRandomBlocks(std::move(storage), amountsSpent, amountsCollected, timeSpacing);
		}

		void AssertNemesisAccount(const cache::AccountStateCacheView& view) {
			auto nemesisKeyPair = crypto::KeyPair::FromString(test::Mijin_Test_Nemesis_Private_Key);
			auto address = model::PublicKeyToAddress(nemesisKeyPair.publicKey(), Network_Identifier);

			auto pNemesisAccountState = view.findAccount(address);
			ASSERT_TRUE(!!pNemesisAccountState);
			EXPECT_EQ(Height(1), pNemesisAccountState->AddressHeight);
			EXPECT_EQ(Height(1), pNemesisAccountState->PublicKeyHeight);
			EXPECT_EQ(0u, pNemesisAccountState->Balances.size());
		}

		void AssertNemesisRecipient(const cache::AccountStateCacheView& view, const Address& address, Amount amountSpent) {
			auto message = model::AddressToString(address);
			auto pAccountState = view.findAccount(address);

			ASSERT_TRUE(!!pAccountState) << message;
			EXPECT_EQ(Height(1), pAccountState->AddressHeight) << message;

			if (Amount(0) != amountSpent)
				EXPECT_LT(Height(0), pAccountState->PublicKeyHeight) << message;

			EXPECT_EQ(Nemesis_Recipient_Amount - amountSpent, pAccountState->Balances.get(Xem_Id)) << message;
		}

		void AssertSecondaryRecipient(const cache::AccountStateCacheView& view, const Address& address, size_t i, Amount amountReceived) {
			auto message = model::AddressToString(address) + " " + std::to_string(i);
			auto pAccountState = view.findAccount(address);

			ASSERT_TRUE(!!pAccountState) << message;
			EXPECT_EQ(Height(i + 2), pAccountState->AddressHeight) << message;
			EXPECT_EQ(Height(0), pAccountState->PublicKeyHeight) << message;
			EXPECT_EQ(amountReceived, pAccountState->Balances.get(Xem_Id)) << message;
		}
	}

	// endregion

	// region ProperAccountCacheState

	namespace {
		void AssertProperAccountCacheStateAfterLoadingMultipleBlocks(const utils::TimeSpan& timeSpacing) {
			// Act:
			TestContext context;
			std::vector<Amount> amountsSpent; // amounts spent by nemesis accounts to send to other newAccounts
			std::vector<Amount> amountsCollected;
			auto newAccounts = PrepareRandomBlocks(std::move(*context.storage()), amountsSpent, amountsCollected, timeSpacing).Recipients;
			context.reboot();

			// Assert:
			auto i = 0u;
			auto cacheView = context.cache().createView();
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
	}

	TEST(LoadBlockChainTests, ProperAccountCacheStateAfterLoadingMultipleBlocks_AllBlocksContributeToTransientState) {
		// Assert:
		AssertProperAccountCacheStateAfterLoadingMultipleBlocks(utils::TimeSpan::FromSeconds(1));
	}

	TEST(LoadBlockChainTests, ProperAccountCacheStateAfterLoadingMultipleBlocks_SomeBlocksContributeToTransientState) {
		// Assert: account state is permanent and should not be short-circuited
		AssertProperAccountCacheStateAfterLoadingMultipleBlocks(utils::TimeSpan::FromMinutes(1));
	}

	// endregion

	// region ProperCacheHeight

	namespace {
		void AssertProperCacheHeightAfterLoadingMultipleBlocks(const utils::TimeSpan& timeSpacing) {
			// Act:
			TestContext context;
			PrepareRandomBlocks(std::move(*context.storage()), timeSpacing);
			context.reboot();

			// Assert:
			auto cacheView = context.cache().createView();
			EXPECT_EQ(Height(Num_Recipient_Accounts + 1), cacheView.height());
		}
	}

	TEST(LoadBlockChainTests, ProperCacheHeightAfterLoadingMultipleBlocks_AllBlocksContributeToTransientState) {
		// Assert:
		AssertProperCacheHeightAfterLoadingMultipleBlocks(utils::TimeSpan::FromSeconds(1));
	}

	TEST(LoadBlockChainTests, ProperCacheHeightAfterLoadingMultipleBlocks_SomeBlocksContributeToTransientState) {
		// Assert: cache height is permanent and should not be short-circuited
		AssertProperCacheHeightAfterLoadingMultipleBlocks(utils::TimeSpan::FromMinutes(1));
	}

	// endregion

	// region ProperChainScore

	namespace {
		void AssertProperChainScoreAfterLoadingMultipleBlocks(const utils::TimeSpan& timeSpacing) {
			// Act:
			TestContext context;
			PrepareRandomBlocks(std::move(*context.storage()), timeSpacing);
			context.reboot();

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

	TEST(LoadBlockChainTests, ProperChainScoreAfterLoadingMultipleBlocks_AllBlocksContributeToTransientState) {
		// Assert:
		AssertProperChainScoreAfterLoadingMultipleBlocks(utils::TimeSpan::FromSeconds(1));
	}

	TEST(LoadBlockChainTests, ProperChainScoreAfterLoadingMultipleBlocks_SomeBlocksContributeToTransientState) {
		// Assert: chain score is permanent and should not be short-circuited
		AssertProperChainScoreAfterLoadingMultipleBlocks(utils::TimeSpan::FromMinutes(1));
	}

	// endregion

	// region ProperTransientCacheState

	namespace {
		template<typename T>
		T Sum(const std::vector<T>& vec, size_t startIndex, size_t endIndex) {
			T sum = 0;
			for (auto i = startIndex; i <= endIndex; ++i)
				sum += vec[i];

			return sum;
		}
	}

	TEST(LoadBlockChainTests, ProperTransientCacheStateAfterLoadingMultipleBlocks_AllBlocksContributeToTransientState) {
		// Act:
		// - note that even though the config is zeroed, MaxTransientStateCacheDuration is 1hr because of the
		//   min RollbackVariabilityBufferDuration
		// - 1s block spacing will sum to much less than 1hr, so state from all blocks should be cached
		TestContext context;
		auto transactionCounts = PrepareRandomBlocks(std::move(*context.storage()), utils::TimeSpan::FromSeconds(1)).TransactionCounts;
		context.reboot();

		auto numTotalTransferTransactions = Sum(transactionCounts, 0, transactionCounts.size() - 1);

		// Assert: all hashes and difficulties were cached
		// - adjust comparisons for the nemesis block, which has
		//   1) Num_Nemesis_Namespaces register namespace transactions
		//   2) for each mosaic one mosaic definition transaction and one mosaic supply change transaction
		//   3) Num_Nemesis_Accounts transfer transactions
		auto cacheView = context.cache().createView();
		EXPECT_EQ(
				numTotalTransferTransactions + Num_Nemesis_Accounts + Num_Nemesis_Namespaces + 2 * Num_Nemesis_Mosaics,
				cacheView.sub<cache::HashCache>().size());

		const auto& blockDifficultyCache = cacheView.sub<cache::BlockDifficultyCache>();
		EXPECT_EQ(transactionCounts.size() + 1, blockDifficultyCache.size());
		EXPECT_EQ(Height(1), blockDifficultyCache.cbegin()->BlockHeight);
		EXPECT_EQ(Height(1 + transactionCounts.size()), std::prev(blockDifficultyCache.cend())->BlockHeight);
	}

	namespace {
		void AssertProperTransientCacheStateAfterLoadingMultipleBlocksWithInflection(
				uint32_t maxDifficultyBlocks,
				size_t numExpectedSignificantBlocks) {
			// Act:
			// - note that even though the config is zeroed, MaxTransientStateCacheDuration is 1hr because of the
			//   min RollbackVariabilityBufferDuration
			// - 1m block spacing will sum to greater than 1hr, so state from some blocks should not be cached
			TestContext context(maxDifficultyBlocks);
			auto transactionCounts = PrepareRandomBlocks(std::move(*context.storage()), utils::TimeSpan::FromMinutes(1)).TransactionCounts;
			context.reboot();

			// Sanity: numExpectedSignificantBlocks should be a subset of all blocks
			ASSERT_LT(numExpectedSignificantBlocks, transactionCounts.size());

			auto startAllObserversIndex = transactionCounts.size() - numExpectedSignificantBlocks;
			auto numTotalTransactions = Sum(transactionCounts, startAllObserversIndex, transactionCounts.size() - 1);

			// Assert: older hashes and difficulties were not cached
			//         (note that transactionCounts indexes 0..N correspond to heights 2..N+2)
			auto cacheView = context.cache().createView();
			EXPECT_EQ(numTotalTransactions, cacheView.sub<cache::HashCache>().size());

			const auto& blockDifficultyCache = cacheView.sub<cache::BlockDifficultyCache>();
			EXPECT_EQ(numExpectedSignificantBlocks, blockDifficultyCache.size());
			EXPECT_EQ(Height(2 + startAllObserversIndex), blockDifficultyCache.cbegin()->BlockHeight);
			EXPECT_EQ(Height(1 + transactionCounts.size()), std::prev(blockDifficultyCache.cend())->BlockHeight);
		}
	}

	TEST(LoadBlockChainTests, ProperTransientCacheStateAfterLoadingMultipleBlocks_SomeBlocksContributeToTransientState_TimeDominant) {
		// Assert: state from blocks at times [T - 60, T] should be cached
		AssertProperTransientCacheStateAfterLoadingMultipleBlocksWithInflection(60, 61);
	}

	TEST(LoadBlockChainTests, ProperTransientCacheStateAfterLoadingMultipleBlocks_SomeBlocksContributeToTransientState_HeightDominant) {
		// Assert: state from the last 75 blocks should be cached
		AssertProperTransientCacheStateAfterLoadingMultipleBlocksWithInflection(75, 75);
	}

	// endregion

	// endregion
}}}
