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

#include "catapult/local/recovery/RecoveryOrchestrator.h"
#include "catapult/cache/SupplementalDataStorage.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/BlockStatisticCache.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/consumers/BlockChainSyncHandlers.h"
#include "catapult/extensions/LocalNodeStateFileStorage.h"
#include "catapult/extensions/NemesisBlockLoader.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "catapult/local/server/FileStateChangeStorage.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "tests/catapult/local/recovery/test/FilechainTestUtils.h"
#include "tests/test/core/BlockStorageTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/StateTestUtils.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/core/TransactionStatusTestUtils.h"
#include "tests/test/local/BlockStateHash.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/MessageIngestionTestContext.h"
#include "tests/test/nemesis/NemesisCompatibleConfiguration.h"
#include "tests/test/other/mocks/MockBlockChangeSubscriber.h"
#include "tests/test/other/mocks/MockBlockHeightCapturingNotificationObserver.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS RecoveryOrchestratorTests

	namespace {
		// region PrepareRandomBlocks

		auto PrepareRandomBlocks(const config::CatapultDirectory& dataDirectory, uint64_t numBlocks) {
			std::vector<uint64_t> scores;
			auto recipients = test::GenerateRandomAddresses(numBlocks);

			// generate block per every recipient, each with random number of transactions
			auto height = 2u;
			std::mt19937_64 rnd;
			auto nemesisKeyPairs = test::GetNemesisKeyPairs();

			// dummy nemesis, just to make score calculations easier
			auto pParentBlock = std::make_unique<model::Block>();
			pParentBlock->Timestamp = Timestamp();

			io::FileBlockStorage storage(dataDirectory.str());
			for (auto i = 0u; i < numBlocks; ++i) {
				auto blockWithAttributes = test::CreateBlock(nemesisKeyPairs, recipients[i], rnd, height, utils::TimeSpan::FromMinutes(1));
				storage.saveBlock(test::BlockToBlockElement(*blockWithAttributes.pBlock));
				++height;

				scores.push_back(chain::CalculateScore(*pParentBlock, *blockWithAttributes.pBlock));
				pParentBlock = std::move(blockWithAttributes.pBlock);
			}

			return scores;
		}

		// endregion

		// region state saving

		cache::SupplementalData CreateSupplementalDataWithDeterministicChainScore() {
			cache::SupplementalData supplementalData;
			supplementalData.ChainScore = model::ChainScore(0x1234567890ABCDEF, 0xFEDCBA0987654321);
			supplementalData.State = test::CreateRandomCatapultState();
			return supplementalData;
		}

		void PopulateBlockStatisticCache(cache::BlockStatisticCacheDelta& cacheDelta, Height startHeight, Height endHeight) {
			for (auto height = startHeight; height <= endHeight; height = height + Height(1)) {
				auto seed = static_cast<uint32_t>(height.unwrap() + 1);
				cacheDelta.insert({ height, Timestamp(2 * seed), Difficulty(3 * seed), BlockFeeMultiplier(4 * seed) });
			}
		}

		void SeedCacheWithNemesis(const extensions::LocalNodeStateRef& stateRef, const plugins::PluginManager& pluginManager) {
			auto cacheDelta = stateRef.Cache.createDelta();
			extensions::NemesisBlockLoader loader(cacheDelta, pluginManager, pluginManager.createObserver());
			loader.executeAndCommit(stateRef, extensions::StateHashVerification::Disabled);
		}

		void RandomSeedCache(cache::CatapultCache& catapultCache, Height cacheHeight, const state::CatapultState& state) {
			// Arrange: seed the cache with random data
			{
				auto delta = catapultCache.createDelta();
				PopulateBlockStatisticCache(delta.sub<cache::BlockStatisticCache>(), Height(2), cacheHeight);
				delta.dependentState() = state;
				catapultCache.commit(cacheHeight);
			}

			// Sanity: data was seeded
			auto view = catapultCache.createView();
			EXPECT_EQ(cacheHeight.unwrap(), view.sub<cache::BlockStatisticCache>().size());
		}

		void PrepareAndSaveState(
				const config::CatapultDirectory& directory,
				plugins::PluginManager& pluginManager,
				bool shouldUseCacheDatabase,
				Height cacheHeight) {
			// Arrange:
			if (Height() == cacheHeight)
				return;

			auto supplementalData = CreateSupplementalDataWithDeterministicChainScore();

			// - seed with nemesis block, so that nemesis accounts have proper balances
			test::LocalNodeTestState state(pluginManager.createCache());
			SeedCacheWithNemesis(state.ref(), pluginManager);
			RandomSeedCache(state.ref().Cache, cacheHeight, supplementalData.State);

			// - save state
			extensions::LocalNodeStateSerializer serializer(config::CatapultDirectory(directory.str()));
			if (shouldUseCacheDatabase) {
				auto storages = const_cast<const cache::CatapultCache&>(state.ref().Cache).storages();
				auto cacheDelta = state.ref().Cache.createDelta();
				serializer.save(cacheDelta, storages, supplementalData.ChainScore, cacheHeight);
			} else {
				serializer.save(state.ref().Cache, supplementalData.ChainScore);
			}
		}

		// endregion

		// region test context

		enum class Flags : uint8_t {
			Default = 0,
			Cache_Database_Enabled = 1
		};

		bool HasFlag(Flags testedFlag, Flags value) {
			return utils::to_underlying_type(testedFlag) == (utils::to_underlying_type(testedFlag) & utils::to_underlying_type(value));
		}

		class RecoveryOrchestratorTestContext : public test::MessageIngestionTestContext {
		public:
			RecoveryOrchestratorTestContext()
					: RecoveryOrchestratorTestContext(Flags::Cache_Database_Enabled, Height(1), Height(0))
			{}

			RecoveryOrchestratorTestContext(Flags flags, Height storageHeight, Height cacheHeight)
					: m_useCacheDatabaseStorage(HasFlag(Flags::Cache_Database_Enabled, flags))
					, m_storageHeight(storageHeight)
					, m_cacheHeight(cacheHeight)
					, m_enableBlockChangeSubscriber(false)
					, m_enableBlockHeightsObserver(false)
			{}

		public:
			void enableBlockChangeSubscriber() {
				m_enableBlockChangeSubscriber = true;
			}

			void enableBlockHeightsObserver() {
				m_enableBlockHeightsObserver = true;
			}

			bool commitStepFileExists() const {
				return commitStepIndexFile().exists();
			}

			consumers::CommitOperationStep readCommitStepFile() {
				return static_cast<consumers::CommitOperationStep>(commitStepIndexFile().get());
			}

			void setCommitStepFile(consumers::CommitOperationStep operationStep) {
				commitStepIndexFile().set(static_cast<uint64_t>(operationStep));
			}

			config::CatapultDirectory subDir(const std::string& name) const {
				return dataDirectory().dir(name);
			}

			boost::filesystem::path spoolDir(const std::string& queueName) const {
				return dataDirectory().spoolDir(queueName).path();
			}

		public:
			const auto& blockScores() const {
				return m_blockScores;
			}

			const auto& blockHeights() const {
				return m_blockHeights;
			}

			const auto& blockStates() const {
				return m_blockStates;
			}

			Height storageHeight() const {
				io::IndexFile indexFile(dataDirectory().rootDir().file("index.dat"));
				return Height(indexFile.get());
			}

			config::CatapultConfiguration createConfig() const {
				auto config = test::CreateCatapultConfigurationWithNemesisPluginExtensions(dataDirectory().rootDir().str());
				if (m_useCacheDatabaseStorage) {
					const_cast<model::BlockChainConfiguration&>(config.BlockChain).EnableVerifiableState = true;
					const_cast<config::NodeConfiguration&>(config.Node).EnableCacheDatabaseStorage = true;
				}

				return config;
			}

			RecoveryOrchestrator& orchestrator() const {
				return *m_pRecoveryOrchestrator;
			}

		private:
			io::IndexFile commitStepIndexFile() const {
				return io::IndexFile(dataDirectory().rootDir().file("commit_step.dat"));
			}

		public:
			void boot() {
				auto config = createConfig();

				// seed the data directory at most once
				const auto& dataDirectoryRoot = dataDirectory().rootDir();
				if (!boost::filesystem::exists(dataDirectoryRoot.path() / "00000")) {
					test::PrepareStorage(dataDirectoryRoot.str());

					if (m_useCacheDatabaseStorage) {
						// calculate the state hash (default nemesis block has zeroed state hash)
						test::ModifyNemesis(dataDirectoryRoot.str(), [&config](auto& nemesisBlock, const auto& nemesisBlockElement) {
							nemesisBlock.StateHash = test::CalculateNemesisStateHash(nemesisBlockElement, config);
						});
					}
				}

				// prepare storage
				prepareSavedStorage(config);

				test::AddRecoveryPluginExtensions(const_cast<config::ExtensionsConfiguration&>(config.Extensions));
				auto pBootstrapper = std::make_unique<extensions::ProcessBootstrapper>(
						std::move(config),
						resourcesDirectory(),
						extensions::ProcessDisposition::Recovery,
						"RecoveryOrchestratorTests");
				pBootstrapper->loadExtensions();

				if (m_enableBlockChangeSubscriber)
					pBootstrapper->subscriptionManager().addBlockChangeSubscriber(std::make_unique<mocks::MockBlockChangeSubscriber>());

				if (m_enableBlockHeightsObserver) {
					pBootstrapper->pluginManager().addObserverHook([this](auto& builder) {
						using ObserverPointer = observers::NotificationObserverPointerT<model::Notification>;
						ObserverPointer pObserver = std::make_unique<const mocks::MockBlockHeightCapturingNotificationObserver>(
								m_blockHeights,
								m_blockStates);
						builder.add(std::move(pObserver));
					});
				}

				m_pRecoveryOrchestrator = CreateRecoveryOrchestrator(std::move(pBootstrapper));
			}

			void reset() {
				m_pRecoveryOrchestrator.reset();
			}

		private:
			void prepareSavedStorage(const config::CatapultConfiguration& config) {
				m_blockScores = PrepareRandomBlocks(dataDirectory().rootDir(), m_storageHeight.unwrap() - 1);
				CATAPULT_LOG(debug) << "storage prepared";

				auto pPluginManager = test::CreatePluginManagerWithRealPlugins(config);
				PrepareAndSaveState(dataDirectory().dir("state"), *pPluginManager, m_useCacheDatabaseStorage, m_cacheHeight);
				CATAPULT_LOG(debug) << "state prepared";
			}

		private:
			bool m_useCacheDatabaseStorage;
			Height m_storageHeight;
			Height m_cacheHeight;
			bool m_enableBlockChangeSubscriber;
			bool m_enableBlockHeightsObserver;
			std::vector<uint64_t> m_blockScores;
			std::vector<Height> m_blockHeights;
			std::vector<state::CatapultState> m_blockStates;
			std::unique_ptr<RecoveryOrchestrator> m_pRecoveryOrchestrator;
		};
	}

	// endregion

	// region basic tests

	TEST(TEST_CLASS, CanBootRecoveryOrchestrator) {
		// Arrange:
		RecoveryOrchestratorTestContext context;

		// Act + Assert: no exeception
		context.boot();
	}

	TEST(TEST_CLASS, CanShutdownRecoveryOrchestrator) {
		// Arrange:
		RecoveryOrchestratorTestContext context;
		context.boot();

		// Act + Assert: no exception
		context.orchestrator().shutdown();
	}

	// endregion

	// region commit step recovery

	TEST(TEST_CLASS, CanRepairCommitStepFile) {
		// Arrange: this test checks that system state is reset
		RecoveryOrchestratorTestContext context;
		context.setCommitStepFile(consumers::CommitOperationStep::Blocks_Written);

		// Sanity:
		EXPECT_TRUE(context.commitStepFileExists());

		// Act:
		context.boot();

		// Assert:
		EXPECT_FALSE(context.commitStepFileExists());
	}

	// endregion

	// region basic spooling recovery

	TEST(TEST_CLASS, CanRepairSpoolingDirectories) {
		// Arrange: create a message file in a spooling directory that should get purged
		RecoveryOrchestratorTestContext context;
		boost::filesystem::create_directories(context.spoolDir("unconfirmed_transactions_change"));
		io::IndexFile((context.spoolDir("unconfirmed_transactions_change") / "message").generic_string()).set(0);

		// Sanity:
		EXPECT_TRUE(boost::filesystem::exists(context.spoolDir("unconfirmed_transactions_change") / "message"));

		// Act:
		context.boot();

		// Assert: the message file was deleted
		EXPECT_FALSE(boost::filesystem::exists(context.spoolDir("unconfirmed_transactions_change") / "message"));
	}

	// endregion

	// region subscriber recovery (ingestion) - traits

	namespace {
		struct BlockChangeTraits {
			static constexpr auto Queue_Directory_Name = "block_change";
			static constexpr auto Num_Expected_Index_Files = 2u;

			static void WriteMessage(io::OutputStream& outputStream) {
				io::Write8(outputStream, utils::to_underlying_type(subscribers::BlockChangeOperationType::Drop_Blocks_After));
				io::Write64(outputStream, test::Random());
			}
		};

		struct TransactionStatusTraits {
			static constexpr auto Queue_Directory_Name = "transaction_status";
			static constexpr auto Num_Expected_Index_Files = 2u;

			static void WriteMessage(io::OutputStream& outputStream) {
				auto notification = test::GenerateRandomTransactionStatusNotification(141);
				test::WriteTransactionStatusNotification(outputStream, notification);
			}
		};

		struct StateChangeTraits {
			static constexpr auto Queue_Directory_Name = "state_change";
			static constexpr auto Num_Expected_Index_Files = 3u;

			static void WriteMessage(io::OutputStream& outputStream) {
				io::Write8(outputStream, utils::to_underlying_type(subscribers::StateChangeOperationType::Score_Change));
				io::Write64(outputStream, test::Random());
				io::Write64(outputStream, test::Random());
			}
		};
	}

#define SUBSCRIBER_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_BlockChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BlockChangeTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_TransactionStatus) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TransactionStatusTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_StateChange) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StateChangeTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region subscriber recovery (ingestion) - tests

	namespace {
		enum class IngestMessagesMode {
			Broker,
			Server
		};

		template<typename TTraits>
		void SetMode(const RecoveryOrchestratorTestContext& context, IngestMessagesMode ingestMessagesMode) {
			auto queuePath = context.spoolDir(TTraits::Queue_Directory_Name);
			boost::filesystem::create_directories(queuePath);
			auto indexName = IngestMessagesMode::Broker == ingestMessagesMode ? "index_broker_r.dat" : "index_server_r.dat";
			io::IndexFile((queuePath / indexName).generic_string()).set(0);
		}
	}

	SUBSCRIBER_TRAITS_BASED_TEST(CanIngestMessagesPresentWhenBooted) {
		// Arrange:
		RecoveryOrchestratorTestContext context;
		context.enableBlockChangeSubscriber();

		// - force index_broker_r.dat (broker mode) to be used by RepairState
		SetMode<TTraits>(context, IngestMessagesMode::Broker);

		// Act + Assert:
		test::ProduceAndConsumeMessages<TTraits>(context, 7, 0, TTraits::Num_Expected_Index_Files);
	}

	SUBSCRIBER_TRAITS_BASED_TEST(CanIngestMessagesPresentWhenBootedStartingAtArbitraryIndex) {
		// Arrange:
		RecoveryOrchestratorTestContext context;
		context.enableBlockChangeSubscriber();

		// - force index_broker_r.dat (broker mode) to be used by RepairState
		SetMode<TTraits>(context, IngestMessagesMode::Broker);

		// - produce and consume some messages
		test::ProduceAndConsumeMessages<TTraits>(context, 7, 0, TTraits::Num_Expected_Index_Files);

		// - reset orchestrator in order to prevent further message digestion
		context.reset();

		// Act + Assert: produce and consume more messages (so ingestion starts at not the first message)
		// (second call to context.boot() will create a new orchestrator)
		test::ProduceAndConsumeMessages<TTraits>(context, 5, 7, TTraits::Num_Expected_Index_Files);
	}

	// endregion

	// region state recovery - blocks

	namespace {
		void SetStorageHeight(const std::string& directory, Height startHeight, Height endHeight) {
			test::PrepareStorageWithoutNemesis(directory);
			test::FakeHeight(directory, startHeight.unwrap());

			io::FileBlockStorage storage(directory);
			for (auto height = startHeight; height <= endHeight; height = height + Height(1)) {
				// create empty blocks to simplify setup required for undo
				auto pBlock = test::GenerateBlockWithTransactions(0, height);
				pBlock->FeeMultiplier = BlockFeeMultiplier(0);
				pBlock->BeneficiaryAddress = model::GetSignerAddress(*pBlock);
				storage.saveBlock(test::BlockToBlockElement(*pBlock, test::GenerateRandomByteArray<Hash256>()));
			}
		}

		void PrepareStateChanges(const RecoveryOrchestratorTestContext& context, Height startHeight, Height endHeight) {
			auto pPluginManager = test::CreatePluginManagerWithRealPlugins(context.createConfig());
			auto catapultCache = pPluginManager->createCache();
			auto cacheDelta = catapultCache.createDelta();

			// 1. add all block signers to account state cache
			io::FileBlockStorage blockStorage(context.spoolDir("block_sync").generic_string());
			for (auto height = startHeight; height <= endHeight; height = height + Height(1)) {
				auto pBlockElement = blockStorage.loadBlockElement(height);
				cacheDelta.sub<cache::AccountStateCache>().addAccount(pBlockElement->Block.SignerPublicKey, height);
			}

			// 2. add statistics for all blocks
			PopulateBlockStatisticCache(cacheDelta.sub<cache::BlockStatisticCache>(), startHeight, endHeight);

			// 3. save all changes to spool directory
			auto pStateChangeStorage = CreateFileStateChangeStorage(
					std::make_unique<io::FileQueueWriter>(context.spoolDir("state_change").generic_string(), "index_server.dat"),
					[&catapultCache]() { return catapultCache.changesStorages(); });
			io::IndexFile((context.spoolDir("state_change") / "index.dat").generic_string()).set(0);
			pStateChangeStorage->notifyStateChange({ cache::CacheChanges(cacheDelta), model::ChainScore(100), endHeight });
		}

		void PrepareSupplementalDataFile(const config::CatapultDirectory& directory) {
			boost::filesystem::create_directories(directory.path());
			auto supplementalDataStream = io::BufferedOutputFileStream(io::RawFile(
					directory.file("supplemental.dat"),
					io::OpenMode::Read_Write));

			cache::SupplementalData supplementalData;
			supplementalData.State.NumTotalTransactions = 998877;
			cache::SaveSupplementalData(supplementalData, Height(), supplementalDataStream);
		}

		auto RunCanMoveBlocksTest(consumers::CommitOperationStep commitStep, Height expectedHeight) {
			// Arrange: seed nemesis block
			RecoveryOrchestratorTestContext context;
			context.enableBlockHeightsObserver();
			context.setCommitStepFile(commitStep);

			// - seed four pending blocks
			SetStorageHeight(context.spoolDir("block_sync").generic_string(), Height(2), Height(5));

			// - prepare cache state changes
			PrepareStateChanges(context, Height(2), Height(5));

			// - prepare supplemental data
			PrepareSupplementalDataFile(context.subDir("state.tmp"));

			// Sanity:
			EXPECT_TRUE(context.commitStepFileExists());
			EXPECT_NE(0u, context.countMessageFiles("block_sync"));
			EXPECT_EQ(1u, context.countMessageFiles("state_change"));

			// Act:
			context.boot();

			// Assert:
			EXPECT_FALSE(context.commitStepFileExists());
			EXPECT_EQ(0u, context.countMessageFiles("block_sync"));
			EXPECT_EQ(expectedHeight, context.storageHeight());

			// - catapult state is loaded from supplemental data (using NumTotalTransactions as sentinel)
			// - first state corresponds to nemesis, which is processed *BEFORE* loading supplemental data
			EXPECT_FALSE(context.blockStates().empty());

			auto i = 0u;
			for (const auto& state : context.blockStates()) {
				EXPECT_EQ(0 == i ? 0u : 998877u, state.NumTotalTransactions);
				++i;
			}

			return context.blockHeights();
		}
	}

	TEST(TEST_CLASS, BlocksAreMovedFromBlockSyncWhenStepIsStateWritten) {
		// Act:
		auto heights = RunCanMoveBlocksTest(consumers::CommitOperationStep::State_Written, Height(5));

		// Assert: nemesis was executed and then all blocks were undone and reapplied
		auto expectedHeights = std::vector<Height>{
			Height(1),
			Height(5), Height(4), Height(3), Height(2),
			Height(2), Height(3), Height(4), Height(5)
		};
		EXPECT_EQ(expectedHeights, heights);
	}

	TEST(TEST_CLASS, BlocksAreNotMovedFromBlockSyncWhenStepIsBlocksWritten) {
		// Act:
		auto heights = RunCanMoveBlocksTest(consumers::CommitOperationStep::Blocks_Written, Height(1));

		// Assert: only nemesis was executed
		auto expectedHeights = std::vector<Height>{ Height(1) };
		EXPECT_EQ(expectedHeights, heights);
	}

	// endregion

	// region state recovery - state change messages

	namespace {
		template<typename TTraits>
		void SetServerBehind(const RecoveryOrchestratorTestContext& context) {
			auto queuePath = context.spoolDir(TTraits::Queue_Directory_Name);
			boost::filesystem::create_directories(queuePath);
			io::IndexFile((queuePath / "index.dat").generic_string()).set(5);
			io::IndexFile((queuePath / "index_server.dat").generic_string()).set(7);
		}

		auto CreateMessageWriter(uint64_t& value1, uint64_t& value2) {
			return [&value1, &value2](auto& outputStream) {
				io::Write8(outputStream, utils::to_underlying_type(subscribers::StateChangeOperationType::Score_Change));
				io::Write64(outputStream, value1);
				io::Write64(outputStream, value2);

				auto temp = value2;
				value2 += value1;
				value1 = temp;
			};
		}

		void AssertCanRepairStateChangeDirectoryWithOutstandingMessages(
				consumers::CommitOperationStep commitStep,
				IngestMessagesMode ingestMessagesMode,
				const consumer<const RecoveryOrchestratorTestContext&>& assertState) {
			// Arrange:
			RecoveryOrchestratorTestContext context;
			context.setCommitStepFile(commitStep);
			context.enableBlockChangeSubscriber();

			// - force index_broker_r.dat|index_server_r.dat to be used by RepairState
			SetMode<StateChangeTraits>(context, ingestMessagesMode);

			// - prepare messages and force index_server.dat to be further than index.dat (server mode)
			uint64_t value1 = 1, value2 = 1;
			context.writeMessages(StateChangeTraits::Queue_Directory_Name, 7, CreateMessageWriter(value1, value2));
			SetServerBehind<StateChangeTraits>(context);

			// Sanity:
			EXPECT_EQ(7u, context.countMessageFiles(StateChangeTraits::Queue_Directory_Name));
			EXPECT_EQ(0u, context.readIndexReaderFile(StateChangeTraits::Queue_Directory_Name, "index_broker_r.dat"));
			EXPECT_EQ(0u, context.readIndexReaderFile(StateChangeTraits::Queue_Directory_Name, "index_server_r.dat"));
			EXPECT_EQ(5u, context.readIndexReaderFile(StateChangeTraits::Queue_Directory_Name, "index.dat"));
			EXPECT_EQ(7u, context.readIndexReaderFile(StateChangeTraits::Queue_Directory_Name, "index_server.dat"));

			// Act:
			context.boot();

			// Assert:
			assertState(context);
		}

		struct ResultsDescriptor {
			size_t NumMessageFiles;
			size_t BrokerReaderIndex;
			size_t ServerReaderIndex;
			size_t MainIndex;
			size_t ServerWriterIndex;
		};

		void AssertIndexesAndNumFiles(const RecoveryOrchestratorTestContext& context, const ResultsDescriptor& expected) {
			// Assert:
			constexpr auto Directory_Name = StateChangeTraits::Queue_Directory_Name;
			EXPECT_EQ(expected.NumMessageFiles, context.countMessageFiles(Directory_Name));
			EXPECT_EQ(StateChangeTraits::Num_Expected_Index_Files, context.countIndexFiles(Directory_Name));

			EXPECT_EQ(expected.BrokerReaderIndex, context.readIndexReaderFile(Directory_Name, "index_broker_r.dat"));
			EXPECT_EQ(expected.ServerReaderIndex, context.readIndexReaderFile(Directory_Name, "index_server_r.dat"));
			EXPECT_EQ(expected.MainIndex, context.readIndexReaderFile(Directory_Name, "index.dat"));
			EXPECT_EQ(expected.ServerWriterIndex, context.readIndexReaderFile(Directory_Name, "index_server.dat"));
		}
	}

	TEST(TEST_CLASS, CanIngestMessagesPresentWhenBootedAndBrokerMode) {
		// Arrange + Act:
		constexpr auto Commit_Step = consumers::CommitOperationStep::State_Written;
		AssertCanRepairStateChangeDirectoryWithOutstandingMessages(Commit_Step, IngestMessagesMode::Broker, [](const auto& context) {
			// Assert:
			// - messages have been consumed (0 left)
			// - broker reader index fully advanced
			// - index.dat fully advanced
			// - WriteMessages writes 7 messages so score will contain fibonacci numbers 7 and 8
			AssertIndexesAndNumFiles(context, { 0, 7, 0, 7, 7 });
			EXPECT_EQ(model::ChainScore(13, 21), context.orchestrator().score());
		});
	}

	TEST(TEST_CLASS, CanIngestMessagesPresentWhenBootedAndServerMode) {
		// Arrange + Act:
		constexpr auto Commit_Step = consumers::CommitOperationStep::State_Written;
		AssertCanRepairStateChangeDirectoryWithOutstandingMessages(Commit_Step, IngestMessagesMode::Server, [](const auto& context) {
			// Assert:
			// - messages have been consumed (0 left)
			// - server reader index fully advanced
			// - index.dat fully advanced
			// - WriteMessages writes 7 messages so score will contain fibonacci numbers 7 and 8
			AssertIndexesAndNumFiles(context, { 0, 0, 7, 7, 7 });
			EXPECT_EQ(model::ChainScore(13, 21), context.orchestrator().score());
		});
	}

	TEST(TEST_CLASS, CanRepairSpoolingDirectoriesWithStateChangeArtifactsAndBrokerMode) {
		// Arrange + Act:
		constexpr auto Commit_Step = consumers::CommitOperationStep::Blocks_Written;
		AssertCanRepairStateChangeDirectoryWithOutstandingMessages(Commit_Step, IngestMessagesMode::Broker, [](const auto& context) {
			// Assert:
			// - messages left untouched (RepairSpooling is not touching them)
			// - server writer index has been reset (during RepairSpooling)
			// - broker reader advanced (5)
			// - nemesis block has been loaded, but no state changes so score equals 1
			AssertIndexesAndNumFiles(context, { 2, 5, 0, 5, 5 });
			EXPECT_EQ(model::ChainScore(1), context.orchestrator().score());
		});
	}

	TEST(TEST_CLASS, CanRepairSpoolingDirectoriesWithStateChangeArtifactsAndServerMode) {
		// Arrange + Act:
		constexpr auto Commit_Step = consumers::CommitOperationStep::Blocks_Written;
		AssertCanRepairStateChangeDirectoryWithOutstandingMessages(Commit_Step, IngestMessagesMode::Server, [](const auto& context) {
			// Assert:
			// - messages left untouched (RepairSpooling is not touching them)
			// - server writer index has been reset (during RepairSpooling)
			// - server reader advanced (5)
			// - nemesis block has been loaded, but no state changes so score equals 1
			AssertIndexesAndNumFiles(context, { 2, 0, 5, 5, 5 });
			EXPECT_EQ(model::ChainScore(1), context.orchestrator().score());
		});
	}

	// endregion

	// region state recovery - supplemental data

	namespace {
		void AssertTemporarySupplementalDataIsPurged(consumers::CommitOperationStep commitStep) {
			// Arrange:
			RecoveryOrchestratorTestContext context;
			context.setCommitStepFile(commitStep);

			// - add temp directory with marker
			auto stateTempDirectory = context.subDir("state.tmp");
			boost::filesystem::create_directories(stateTempDirectory.path());
			io::IndexFile(stateTempDirectory.file("marker.dat")).set(123);

			// Sanity:
			EXPECT_EQ(1u, test::CountFilesAndDirectories(stateTempDirectory.path()));

			// Act:
			context.boot();
			context.reset();

			// Assert: files were purged but not moved
			auto stateDirectory = context.subDir("state");
			EXPECT_FALSE(boost::filesystem::exists(stateTempDirectory.path()));
			EXPECT_TRUE(boost::filesystem::exists(stateDirectory.path()));

			EXPECT_EQ(consumers::CommitOperationStep::All_Updated, context.readCommitStepFile());
		}
	}

	TEST(TEST_CLASS, TemporarySupplementalDataIsPurgedWhenCommitStepIsBlocksWritten) {
		AssertTemporarySupplementalDataIsPurged(consumers::CommitOperationStep::Blocks_Written);
	}

	TEST(TEST_CLASS, TemporarySupplementalDataIsPurgedWhenCommitStepIsAllUpdated) {
		AssertTemporarySupplementalDataIsPurged(consumers::CommitOperationStep::All_Updated);
	}

	TEST(TEST_CLASS, TemporarySupplementalDataIsRecoveredWhenCommitStepIsStateWritten) {
		// Arrange:
		RecoveryOrchestratorTestContext context;
		context.setCommitStepFile(consumers::CommitOperationStep::State_Written);

		// - add temp directory with marker and supplemental file
		auto stateTempDirectory = context.subDir("state.tmp");
		boost::filesystem::create_directories(stateTempDirectory.path());
		io::IndexFile(stateTempDirectory.file("marker.dat")).set(123);
		io::IndexFile(stateTempDirectory.file("supplemental.dat")).set(999);

		// Sanity:
		EXPECT_EQ(2u, test::CountFilesAndDirectories(stateTempDirectory.path()));

		// Act:
		context.boot();
		context.reset();

		// Assert: files were purged and moved
		// - supplemental.dat from state.tmp should take precedence and it should not be written out again
		auto stateDirectory = context.subDir("state");
		EXPECT_FALSE(boost::filesystem::exists(stateTempDirectory.path()));
		EXPECT_EQ(2u, test::CountFilesAndDirectories(stateDirectory.path()));
		EXPECT_EQ(123u, io::IndexFile(stateDirectory.file("marker.dat")).get());
		EXPECT_EQ(999u, io::IndexFile(stateDirectory.file("supplemental.dat")).get());

		EXPECT_EQ(consumers::CommitOperationStep::All_Updated, context.readCommitStepFile());
	}

	// endregion

	// region state loading - cache database disabled

	namespace {
		void AssertLoadChainTest(Height cacheHeight, Height storageHeight) {
			// Arrange:
			RecoveryOrchestratorTestContext context(Flags::Default, storageHeight, cacheHeight);
			context.enableBlockHeightsObserver();

			// Act:
			context.boot();

			// Assert: proper number of blocks were loaded
			const auto& heights = context.blockHeights();
			ASSERT_EQ((storageHeight - cacheHeight).unwrap(), heights.size());

			auto i = 0u;
			auto startHeight = cacheHeight + Height(1);
			for (auto height = startHeight; height <= storageHeight; height = height + Height(1)) {
				EXPECT_EQ(height, heights[i]);
				++i;
			}

			const auto& blockScores = context.blockScores();
			if (Height() == cacheHeight) {
				// - if full chain is loaded, nemesis score is 1 and all blocks contribute to score
				auto expectedScore = model::ChainScore(1);
				for (auto blockScore : blockScores)
					expectedScore += model::ChainScore(blockScore);

				EXPECT_EQ(expectedScore, context.orchestrator().score());
			} else {
				// - if state has been partially loaded, score is faked and some blocks contribute to score
				// `-2` in loop below because blockScores[0] is score for block with height 2
				auto expectedScore = model::ChainScore(0x1234567890ABCDEF, 0xFEDCBA0987654321);
				for (auto height = cacheHeight + Height(1); height <= storageHeight; height = height + Height(1))
					expectedScore += model::ChainScore(blockScores[height.unwrap() - 2]);

				EXPECT_EQ(expectedScore, context.orchestrator().score());
			}
		}

		void AssertThrowsLoadChainTest(Flags flags, Height cacheHeight, Height storageHeight) {
			// Arrange:
			RecoveryOrchestratorTestContext context(flags, storageHeight, cacheHeight);
			context.enableBlockHeightsObserver();

			// Act + Assert:
			EXPECT_THROW(context.boot(), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, CanLoadChainWhenCacheIsEmptyAndOnlyNemesisIsInStorage_CacheDatabaseDisabled) {
		AssertLoadChainTest(Height(0), Height(1));
	}

	TEST(TEST_CLASS, CanLoadChainWhenCacheIsEmptyAndMultipleBlocksInStorage_CacheDatabaseDisabled) {
		AssertLoadChainTest(Height(0), Height(4));
	}

	TEST(TEST_CLASS, CanLoadChainWhenCacheHeightIsLessThanStorageHeight_CacheDatabaseDisabled) {
		AssertLoadChainTest(Height(4), Height(6));
	}

	TEST(TEST_CLASS, CanLoadChainWhenCacheHeightIsEqualToStorageHeight_CacheDatabaseDisabled) {
		AssertLoadChainTest(Height(6), Height(6));
	}

	TEST(TEST_CLASS, LoadingChainThrowsWhenCacheHeightIsGreaterThanStorageHeight_CacheDatabaseDisabled) {
		AssertThrowsLoadChainTest(Flags::Default, Height(8), Height(6));
	}

	// endregion

	// region state loading - cache database enabled

	namespace {
		void AssertLoadsOnlyNemesis(Height cacheHeight, Height storageHeight) {
			// Arrange:
			RecoveryOrchestratorTestContext context(Flags::Cache_Database_Enabled, storageHeight, cacheHeight);
			context.enableBlockHeightsObserver();

			// Act:
			context.boot();

			// Assert: no blocks were loaded from storage
			const auto& heights = context.blockHeights();
			ASSERT_EQ(1u, heights.size());
			EXPECT_EQ(Height(1), heights[0]);

			// - no blocks are loaded from storage, so only nemesis block contributes to score
			EXPECT_EQ(model::ChainScore(1), context.orchestrator().score());
		}

		void AssertLoadsFromState(Height cacheHeight, Height storageHeight) {
			// Arrange:
			RecoveryOrchestratorTestContext context(Flags::Cache_Database_Enabled, storageHeight, cacheHeight);
			context.enableBlockHeightsObserver();

			// Act:
			context.boot();

			// Assert: no blocks were loaded from storage
			const auto& heights = context.blockHeights();
			ASSERT_EQ(0u, heights.size());

			// - no blocks are loaded from storage, so score is always the same
			EXPECT_EQ(model::ChainScore(0x1234567890ABCDEF, 0xFEDCBA0987654321), context.orchestrator().score());
		}
	}

	TEST(TEST_CLASS, CanLoadChainWhenCacheIsEmptyAndOnlyNemesisIsInStorage_CacheDatabaseEnabled) {
		AssertLoadsOnlyNemesis(Height(0), Height(1));
	}

	TEST(TEST_CLASS, LoadsOnlyNemesisWhenCacheIsEmptyAndMultipleBlocksInStorage_CacheDatabaseEnabled) {
		AssertLoadsOnlyNemesis(Height(0), Height(4));
	}

	TEST(TEST_CLASS, CanLoadChainWhenCacheHeightIsLessThanStorageHeight_CacheDatabaseEnabled) {
		AssertLoadsFromState(Height(4), Height(6));
	}

	TEST(TEST_CLASS, CanLoadChainWhenCacheHeightIsEqualToStorageHeight_CacheDatabaseEnabled) {
		AssertLoadsFromState(Height(6), Height(6));
	}

	TEST(TEST_CLASS, LoadingChainThrowsWhenCacheHeightIsGreaterThanStorageHeight_CacheDatabaseEnabled) {
		AssertThrowsLoadChainTest(Flags::Cache_Database_Enabled, Height(8), Height(6));
	}

	// endregion
}}
