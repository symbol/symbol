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

#include "sync/src/DispatcherService.h"
#include "sdk/src/extensions/TransactionExtensions.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/io/IndexFile.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/plugins/PluginLoader.h"
#include "catapult/utils/NetworkTime.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/other/mocks/MockNotificationValidator.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

using ValidationResult = catapult::validators::ValidationResult;

namespace catapult { namespace sync {

#define TEST_CLASS DispatcherServiceTests

	namespace {
		constexpr auto Num_Expected_Services = 5u;
		constexpr auto Num_Expected_Counters = 8u;
		constexpr auto Num_Expected_Tasks = 1u;

		constexpr auto Block_Elements_Counter_Name = "BLK ELEM TOT";
		constexpr auto Transaction_Elements_Counter_Name = "TX ELEM TOT";
		constexpr auto Block_Elements_Active_Counter_Name = "BLK ELEM ACT";
		constexpr auto Transaction_Elements_Active_Counter_Name = "TX ELEM ACT";
		constexpr auto Rollback_Elements_Committed_All = "RB COMMIT ALL";
		constexpr auto Rollback_Elements_Committed_Recent = "RB COMMIT RCT";
		constexpr auto Rollback_Elements_Ignored_All = "RB IGNORE ALL";
		constexpr auto Rollback_Elements_Ignored_Recent = "RB IGNORE RCT";
		constexpr auto Sentinel_Counter_Value = extensions::ServiceLocator::Sentinel_Counter_Value;

		// region utils

		crypto::KeyPair GetBlockSignerKeyPair() {
			// one requirement of CanConsumeBlockRangeCompletionAware_ValidElement is that the block must hit
			// so, use a fixed account that will generate a hit lower than the target, which is fixed in the test
			// since the cache is setup in InitializeCatapultCacheForDispatcherTests,
			// this account does not need to be a 'real' nemesis account
			return crypto::KeyPair::FromString("75C1A1762304FE9EC59C5E9632D8E88C746BDB92B0563CCDDFC4A15C7BFD4578");
		}

		cache::CatapultCache CreateCatapultCacheForDispatcherTests() {
			// importance grouping must be non-zero
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.ImportanceGrouping = 1;

			// create the cache
			return test::CreateEmptyCatapultCache<test::CoreSystemCacheFactory>(config);
		}

		void InitializeCatapultCacheForDispatcherTests(cache::CatapultCache& cache, const crypto::KeyPair& signer) {
			// create the delta
			auto delta = cache.createDelta();

			// set a difficulty for the nemesis block
			delta.sub<cache::BlockDifficultyCache>().insert(Height(1), Timestamp(0), Difficulty());

			// add a balance and importance for the signer
			auto& accountStateCache = delta.sub<cache::AccountStateCache>();
			accountStateCache.addAccount(signer.publicKey(), Height(1));
			auto& accountState = accountStateCache.find(signer.publicKey()).get();
			accountState.ImportanceInfo.set(Importance(1'000'000'000), model::ImportanceHeight(1));

			// commit all changes
			cache.commit(Height(1));
		}

		std::shared_ptr<model::Block> CreateValidBlockForDispatcherTests(const crypto::KeyPair& signer) {
			constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

			mocks::MockMemoryBlockStorage storage;
			auto pNemesisBlockElement = storage.loadBlockElement(Height(1));

			model::PreviousBlockContext context(*pNemesisBlockElement);
			auto pBlock = model::CreateBlock(context, Network_Identifier, signer.publicKey(), model::Transactions());
			pBlock->Timestamp = context.Timestamp + Timestamp(60000);
			return std::move(pBlock);
		}

		// endregion

		// region MockReceiptBlockObserver

		class MockReceiptBlockObserver : public observers::NotificationObserverT<model::BlockNotification> {
		public:
			MockReceiptBlockObserver() : m_name("MockReceiptBlockObserver")
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			void notify(const model::BlockNotification& notification, observers::ObserverContext& context) const override {
				model::Receipt receipt{};
				receipt.Size = sizeof(model::Receipt);
				receipt.Type = static_cast<model::ReceiptType>(notification.Timestamp.unwrap());
				context.StatementBuilder().addReceipt(receipt);
			}

		private:
			std::string m_name;
		};

		// endregion

		// region TestContext

		struct DispatcherServiceTraits {
			static constexpr auto CreateRegistrar = CreateDispatcherServiceRegistrar;
		};

		struct ValidationResults {
		public:
			ValidationResults() : ValidationResults(ValidationResult::Success, ValidationResult::Success)
			{}

			explicit ValidationResults(ValidationResult statelessResult, ValidationResult statefulResult)
					: Stateless(statelessResult)
					, Stateful(statefulResult)
			{}

		public:
			ValidationResult Stateless;
			ValidationResult Stateful;
		};

		class TestContext : public test::ServiceLocatorTestContext<DispatcherServiceTraits> {
		private:
			using BaseType = test::ServiceLocatorTestContext<DispatcherServiceTraits>;

		public:
			TestContext()
					: BaseType(CreateCatapultCacheForDispatcherTests())
					, m_numNewBlockSinkCalls(0)
					, m_numNewTransactionsSinkCalls(0)
					, m_pStatefulBlockValidator(nullptr) {
				// override data directory
				auto& state = testState().state();
				const_cast<std::string&>(state.config().User.DataDirectory) = m_tempDir.name();

				// initialize the cache
				InitializeCatapultCacheForDispatcherTests(state.cache(), GetBlockSignerKeyPair());

				// set up sinks
				state.hooks().addNewBlockSink([&counter = m_numNewBlockSinkCalls](const auto&) { ++counter; });
				state.hooks().addNewTransactionsSink([&counter = m_numNewTransactionsSinkCalls](const auto&) { ++counter; });

				// the service needs to be able to raise notifications from the mock transactions sent to it
				auto& pluginManager = testState().pluginManager();
				pluginManager.addTransactionSupport(mocks::CreateMockTransactionPlugin());

				// add transaction validators to emulate transaction validation failures
				pluginManager.addStatelessValidatorHook([&result = m_transactionValidationResults.Stateless](auto& builder) {
					auto pValidator = std::make_unique<mocks::MockStatelessNotificationValidatorT<model::TransactionNotification>>();
					pValidator->setResult(result);
					builder.template add<model::TransactionNotification>(std::move(pValidator));
				});
				pluginManager.addStatefulValidatorHook([&result = m_transactionValidationResults.Stateful](auto& builder) {
					auto pValidator = std::make_unique<mocks::MockNotificationValidatorT<model::TransactionNotification>>();
					pValidator->setResult(result);
					builder.template add<model::TransactionNotification>(std::move(pValidator));
				});

				// add block validators to emulate block validation failures
				pluginManager.addStatelessValidatorHook([&result = m_blockValidationResults.Stateless](auto& builder) {
					auto pValidator = std::make_unique<mocks::MockStatelessNotificationValidatorT<model::BlockNotification>>();
					pValidator->setResult(result);
					builder.template add<model::BlockNotification>(std::move(pValidator));
				});
				pluginManager.addStatefulValidatorHook([this, &result = m_blockValidationResults.Stateful](auto& builder) {
					auto pValidator = std::make_unique<mocks::MockNotificationValidatorT<model::BlockNotification>>();
					pValidator->setResult(result);
					m_pStatefulBlockValidator = pValidator.get();
					builder.template add<model::BlockNotification>(std::move(pValidator));
				});

				// add receipt block observer to make sure block receipts hash checks are wired up properly
				pluginManager.addObserverHook([](auto& builder) {
					builder.template add<model::BlockNotification>(std::make_unique<MockReceiptBlockObserver>());
				});
			}

		public:
			boost::filesystem::path tempPath() const {
				return m_tempDir.name();
			}

			std::pair<consumers::CommitOperationStep, bool> tryReadCommitStep() const {
				auto indexFile = io::IndexFile((tempPath() / "commit_step.dat").generic_string());
				return indexFile.exists()
						? std::make_pair(static_cast<consumers::CommitOperationStep>(indexFile.get()), true)
						: std::make_pair(static_cast<consumers::CommitOperationStep>(-1), false);
			}

		public:
			size_t numTransactionStatuses() const {
				return testState().transactionStatusSubscriber().numNotifies();
			}

			size_t numNewBlockSinkCalls() const {
				return m_numNewBlockSinkCalls;
			}

			size_t numNewTransactionsSinkCalls() const {
				return m_numNewTransactionsSinkCalls;
			}

			size_t numStatefulBlockValidatorCalls() const {
				return m_pStatefulBlockValidator->notificationTypes().size();
			}

		public:
			void setTransactionValidationResults(const ValidationResults& transactionValidationResults) {
				m_transactionValidationResults = transactionValidationResults;
			}

			void setBlockValidationResults(const ValidationResults& blockValidationResults) {
				m_blockValidationResults = blockValidationResults;
			}

			void setStatefulBlockValidationResult(ValidationResult result) {
				m_pStatefulBlockValidator->setResult(result);
			}

		private:
			test::TempDirectoryGuard m_tempDir;

			ValidationResults m_transactionValidationResults;
			ValidationResults m_blockValidationResults;
			std::atomic<size_t> m_numNewBlockSinkCalls;
			std::atomic<size_t> m_numNewTransactionsSinkCalls;

			// stateful block validator needs to be captured in order to allow custom rollback tests,
			// which require validator to return different results on demand
			mocks::MockNotificationValidatorT<model::BlockNotification>* m_pStatefulBlockValidator;
		};

		// endregion

		// region DispatcherStatus

		struct DispatcherStatus {
			std::string Name;
			size_t Size;
			bool IsRunning;
		};

		DispatcherStatus GetDispatcherStatus(const std::shared_ptr<disruptor::ConsumerDispatcher>& pDispatcher) {
			return !pDispatcher
					? DispatcherStatus()
					: DispatcherStatus{ pDispatcher->name(), pDispatcher->size(), pDispatcher->isRunning() };
		}

		DispatcherStatus GetBlockDispatcherStatus(const extensions::ServiceLocator& locator) {
			return GetDispatcherStatus(locator.service<disruptor::ConsumerDispatcher>("dispatcher.block"));
		}

		DispatcherStatus GetTransactionDispatcherStatus(const extensions::ServiceLocator& locator) {
			return GetDispatcherStatus(locator.service<disruptor::ConsumerDispatcher>("dispatcher.transaction"));
		}

		// endregion
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(Dispatcher, Post_Remote_Peers)

	// region boot + shutdown

	TEST(TEST_CLASS, CanBootService) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(Num_Expected_Services, context.locator().numServices());
		EXPECT_EQ(Num_Expected_Counters, context.locator().counters().size());
		EXPECT_EQ(Num_Expected_Tasks, context.testState().state().tasks().size());

		// - all services should exist
		EXPECT_TRUE(!!context.locator().service<disruptor::ConsumerDispatcher>("dispatcher.block"));
		EXPECT_TRUE(!!context.locator().service<disruptor::ConsumerDispatcher>("dispatcher.transaction"));
		EXPECT_TRUE(!!context.locator().service<void>("dispatcher.transaction.batch"));
		EXPECT_TRUE(!!context.locator().service<void>("dispatcher.utUpdater"));
		EXPECT_TRUE(!!context.locator().service<void>("rollbacks"));

		// - all counters should be zero
		EXPECT_EQ(0u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));
		EXPECT_EQ(0u, context.counter(Block_Elements_Active_Counter_Name));
		EXPECT_EQ(0u, context.counter(Transaction_Elements_Active_Counter_Name));
		EXPECT_EQ(0u, context.counter(Rollback_Elements_Committed_All));
		EXPECT_EQ(0u, context.counter(Rollback_Elements_Committed_Recent));
		EXPECT_EQ(0u, context.counter(Rollback_Elements_Ignored_All));
		EXPECT_EQ(0u, context.counter(Rollback_Elements_Ignored_Recent));

		// - block dispatcher should be initialized
		auto blockDispatcherStatus = GetBlockDispatcherStatus(context.locator());
		EXPECT_EQ("block dispatcher", blockDispatcherStatus.Name);
		EXPECT_EQ(6u, blockDispatcherStatus.Size);
		EXPECT_TRUE(blockDispatcherStatus.IsRunning);

		// - transaction dispatcher should be initialized
		auto transactionDispatcherStatus = GetTransactionDispatcherStatus(context.locator());
		EXPECT_EQ("transaction dispatcher", transactionDispatcherStatus.Name);
		EXPECT_EQ(4u, transactionDispatcherStatus.Size);
		EXPECT_TRUE(transactionDispatcherStatus.IsRunning);
	}

	TEST(TEST_CLASS, CanBootServiceWithAuditingEnabled) {
		// Arrange: enable auditing
		TestContext context;
		const_cast<bool&>(context.testState().config().Node.ShouldAuditDispatcherInputs) = true;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(Num_Expected_Services, context.locator().numServices());
		EXPECT_EQ(Num_Expected_Counters, context.locator().counters().size());
		EXPECT_EQ(Num_Expected_Tasks, context.testState().state().tasks().size());

		EXPECT_EQ(7u, GetBlockDispatcherStatus(context.locator()).Size);
		EXPECT_EQ(5u, GetTransactionDispatcherStatus(context.locator()).Size);

		// - auditing directories were created
		auto auditDirectory = context.tempPath() / "audit";
		EXPECT_TRUE(boost::filesystem::is_directory(auditDirectory / "block dispatcher"));
		EXPECT_TRUE(boost::filesystem::is_directory(auditDirectory / "transaction dispatcher"));
	}

	TEST(TEST_CLASS, CanBootServiceWithAutoSyncCleanupEnabled) {
		// Arrange: enable cleanup
		TestContext context;
		const_cast<bool&>(context.testState().config().Node.ShouldEnableAutoSyncCleanup) = true;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(Num_Expected_Services, context.locator().numServices());
		EXPECT_EQ(Num_Expected_Counters, context.locator().counters().size());
		EXPECT_EQ(Num_Expected_Tasks, context.testState().state().tasks().size());

		EXPECT_EQ(7u, GetBlockDispatcherStatus(context.locator()).Size);
		EXPECT_EQ(4u, GetTransactionDispatcherStatus(context.locator()).Size);
	}

	TEST(TEST_CLASS, CanShutdownService) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();
		context.shutdown();

		// Assert:
		EXPECT_EQ(Num_Expected_Services, context.locator().numServices());
		EXPECT_EQ(Num_Expected_Counters, context.locator().counters().size());
		EXPECT_EQ(Num_Expected_Tasks, context.testState().state().tasks().size());

		// - only rooted services exist
		EXPECT_FALSE(!!context.locator().service<disruptor::ConsumerDispatcher>("dispatcher.block"));
		EXPECT_FALSE(!!context.locator().service<disruptor::ConsumerDispatcher>("dispatcher.transaction"));
		EXPECT_TRUE(!!context.locator().service<void>("dispatcher.transaction.batch"));
		EXPECT_TRUE(!!context.locator().service<void>("dispatcher.utUpdater"));
		EXPECT_TRUE(!!context.locator().service<void>("rollbacks"));

		// - all counters should indicate shutdown
		EXPECT_EQ(Sentinel_Counter_Value, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(Sentinel_Counter_Value, context.counter(Transaction_Elements_Counter_Name));
		EXPECT_EQ(Sentinel_Counter_Value, context.counter(Block_Elements_Active_Counter_Name));
		EXPECT_EQ(Sentinel_Counter_Value, context.counter(Transaction_Elements_Active_Counter_Name));
		EXPECT_EQ(0u, context.counter(Rollback_Elements_Committed_All));
		EXPECT_EQ(0u, context.counter(Rollback_Elements_Committed_Recent));
		EXPECT_EQ(0u, context.counter(Rollback_Elements_Ignored_All));
		EXPECT_EQ(0u, context.counter(Rollback_Elements_Ignored_Recent));
	}

	// endregion

	// region consume - block range

	namespace {
		template<typename THandler>
		void AssertCanConsumeBlockRange(bool shouldEnableVerifiableReceipts, model::AnnotatedBlockRange&& range, THandler handler) {
			// Arrange:
			TestContext context;
			const auto& blockChainConfig = context.testState().config().BlockChain;
			const_cast<model::BlockChainConfiguration&>(blockChainConfig).ShouldEnableVerifiableReceipts = shouldEnableVerifiableReceipts;

			context.boot();
			auto factory = context.testState().state().hooks().blockRangeConsumerFactory()(disruptor::InputSource::Local);

			// Sanity: commit step index file does not exist
			EXPECT_FALSE(context.tryReadCommitStep().second);

			// Act:
			factory(std::move(range));
			WAIT_FOR_ONE_EXPR(context.counter(Block_Elements_Counter_Name));

			// - wait a bit to give the service time to consume more if there is a bug in the implementation
			test::Pause();

			// Assert:
			EXPECT_EQ(1u, context.counter(Block_Elements_Counter_Name));
			EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));
			handler(context);
		}

		template<typename THandler>
		void AssertCanConsumeBlockRange(model::AnnotatedBlockRange&& range, THandler handler) {
			AssertCanConsumeBlockRange(false, std::move(range), handler);
		}

		void AssertBlockRangeInvalidElement(const TestContext& context) {
			// Assert: the block was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());

			// - commit step index file does not exist
			EXPECT_FALSE(context.tryReadCommitStep().second);
		}

		void AssertCommitStepFileUpdated(const TestContext& context) {
			// Assert:
			auto pair = context.tryReadCommitStep();
			EXPECT_TRUE(pair.second);
			EXPECT_EQ(consumers::CommitOperationStep::All_Updated, pair.first);
		}
	}

	TEST(TEST_CLASS, CanConsumeBlockRange_InvalidElement) {
		// Assert:
		AssertCanConsumeBlockRange(test::CreateBlockEntityRange(1), AssertBlockRangeInvalidElement);
	}

	TEST(TEST_CLASS, CanConsumeBlockRange_ValidElement) {
		// Arrange:
		auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// Assert:
		AssertCanConsumeBlockRange(std::move(range), [](const auto& context) {
			WAIT_FOR_ONE_EXPR(context.numNewBlockSinkCalls());

			// - the block was forwarded to the sink
			EXPECT_EQ(1u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());

			// - commit step index file was updated
			AssertCommitStepFileUpdated(context);
		});
	}

	TEST(TEST_CLASS, CanConsumeBlockRange_InvalidElement_WithVerifiableReceipts) {
		// Arrange: block does not contain correct block receipts hash, so should get rejected
		auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// Assert:
		AssertCanConsumeBlockRange(true, std::move(range), AssertBlockRangeInvalidElement);
	}

	namespace {
		void SetBlockReceiptsHash(model::Block& block) {
			// Arrange: set block receipts hash
			model::Receipt receipt{};
			receipt.Size = sizeof(model::Receipt);
			receipt.Type = static_cast<model::ReceiptType>(block.Timestamp.unwrap());
			model::BlockStatementBuilder blockStatementBuilder;
			blockStatementBuilder.addReceipt(receipt);
			block.BlockReceiptsHash = model::CalculateMerkleHash(*blockStatementBuilder.build());
		}
	}

	TEST(TEST_CLASS, CanConsumeBlockRange_ValidElement_WithVerifiableReceipts) {
		// Arrange: block contains correct block receipts hash, so should get accepted
		auto signer = GetBlockSignerKeyPair();
		auto pNextBlock = CreateValidBlockForDispatcherTests(signer);
		SetBlockReceiptsHash(*pNextBlock);
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// Assert:
		AssertCanConsumeBlockRange(true, std::move(range), [](const auto& context) {
			WAIT_FOR_ONE_EXPR(context.numNewBlockSinkCalls());

			// - the block was forwarded to the sink
			EXPECT_EQ(1u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());

			// - commit step index file was updated
			AssertCommitStepFileUpdated(context);
		});
	}

	// endregion

	// region consume - block range completion aware

	namespace {
		template<typename THandler>
		void AssertCanConsumeBlockRangeCompletionAware(
				const ValidationResults& blockValidationResults,
				model::AnnotatedBlockRange&& range,
				THandler handler) {
			// Arrange:
			TestContext context;
			context.setBlockValidationResults(blockValidationResults);
			context.boot();
			auto factory = context.testState().state().hooks().completionAwareBlockRangeConsumerFactory()(disruptor::InputSource::Local);

			// Sanity: commit step index file does not exist
			EXPECT_FALSE(context.tryReadCommitStep().second);

			// Act:
			std::atomic<size_t> numCompletions(0);
			factory(std::move(range), [&numCompletions](auto, auto) { ++numCompletions; });
			WAIT_FOR_ONE(numCompletions);

			// - wait a bit to give the service time to consume more if there is a bug in the implementation
			test::Pause();

			// Assert:
			EXPECT_EQ(1u, numCompletions);
			EXPECT_EQ(1u, context.counter(Block_Elements_Counter_Name));
			EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));
			handler(context);
		}

		void AssertBlockRangeCompletionAwareInvalidElement(const TestContext& context) {
			// Assert: the block was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());

			// - state change subscriber shouldn't have been called for an invalid element, so chain score should be unchanged (zero)
			const auto& stateChangeSubscriber = context.testState().stateChangeSubscriber();
			EXPECT_EQ(0u, stateChangeSubscriber.numScoreChanges());
			EXPECT_EQ(0u, stateChangeSubscriber.numStateChanges());
			EXPECT_EQ(model::ChainScore(), stateChangeSubscriber.lastChainScore());

			// - commit step index file does not exist
			EXPECT_FALSE(context.tryReadCommitStep().second);
		}
	}

	TEST(TEST_CLASS, CanConsumeBlockRangeCompletionAware_InvalidElement) {
		// Assert: BlockTransactionsHash mismatch
		AssertCanConsumeBlockRangeCompletionAware(
				ValidationResults(),
				test::CreateBlockEntityRange(1),
				AssertBlockRangeCompletionAwareInvalidElement);
	}

	TEST(TEST_CLASS, CanConsumeBlockRangeCompletionAware_InvalidElement_Stateless) {
		// Arrange:
		auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// - stateless failure
		ValidationResults blockValidationResults;
		blockValidationResults.Stateless = ValidationResult::Failure;
		AssertCanConsumeBlockRangeCompletionAware(blockValidationResults, std::move(range), AssertBlockRangeCompletionAwareInvalidElement);
	}

	TEST(TEST_CLASS, CanConsumeBlockRangeCompletionAware_InvalidElement_Stateful) {
		// Arrange:
		auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// - stateful failure
		ValidationResults blockValidationResults;
		blockValidationResults.Stateful = ValidationResult::Failure;
		AssertCanConsumeBlockRangeCompletionAware(blockValidationResults, std::move(range), AssertBlockRangeCompletionAwareInvalidElement);
	}

	TEST(TEST_CLASS, CanConsumeBlockRangeCompletionAware_ValidElement) {
		// Arrange:
		auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// Assert:
		AssertCanConsumeBlockRangeCompletionAware(ValidationResults(), std::move(range), [](const auto& context) {
			// - the block was forwarded to the sink
			EXPECT_EQ(1u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());

			// - state change subscriber should have been called, so chain score should be changed (non-zero)
			const auto& stateChangeSubscriber = context.testState().stateChangeSubscriber();
			EXPECT_EQ(1u, stateChangeSubscriber.numScoreChanges());
			EXPECT_EQ(1u, stateChangeSubscriber.numStateChanges());
			EXPECT_EQ(model::ChainScore(99'999'999'999'940), stateChangeSubscriber.lastChainScore());

			// - commit step index file was updated
			AssertCommitStepFileUpdated(context);
		});
	}

	// endregion

	// region consume - block range rollback

	TEST(TEST_CLASS, SuccessfulRollbackChangesCommittedCounters) {
		// Arrange: create a chain with one block on top of nemesis
		TestContext context;
		context.boot();
		auto keyPair = GetBlockSignerKeyPair();
		auto pBaseBlock = CreateValidBlockForDispatcherTests(keyPair);
		auto factory = context.testState().state().hooks().blockRangeConsumerFactory()(disruptor::InputSource::Local);

		factory(test::CreateEntityRange({ pBaseBlock.get() }));
		WAIT_FOR_ONE_EXPR(context.numNewBlockSinkCalls());

		// Act: create a better block to cause a rollback
		auto pBetterBlock = CreateValidBlockForDispatcherTests(keyPair);
		pBetterBlock->Timestamp = pBetterBlock->Timestamp - Timestamp(1000);

		factory(test::CreateEntityRange({ pBetterBlock.get() }));
		WAIT_FOR_ONE_EXPR(context.counter(Rollback_Elements_Committed_All));
		WAIT_FOR_ZERO_EXPR(context.counter(Block_Elements_Active_Counter_Name));

		// Assert:
		EXPECT_EQ(2u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));

		EXPECT_EQ(1u, context.counter(Rollback_Elements_Committed_All));
		EXPECT_EQ(1u, context.counter(Rollback_Elements_Committed_Recent));
		EXPECT_EQ(0u, context.counter(Rollback_Elements_Ignored_All));
		EXPECT_EQ(0u, context.counter(Rollback_Elements_Ignored_Recent));
	}

	TEST(TEST_CLASS, FailedRollbackChangesIgnoredCounters) {
		// Arrange: create a chain with one block on top of nemesis
		TestContext context;
		context.boot();
		auto keyPair = GetBlockSignerKeyPair();
		auto pBaseBlock = CreateValidBlockForDispatcherTests(keyPair);
		auto factory = context.testState().state().hooks().blockRangeConsumerFactory()(disruptor::InputSource::Local);

		factory(test::CreateEntityRange({ pBaseBlock.get() }));
		WAIT_FOR_ONE_EXPR(context.numNewBlockSinkCalls());

		// Act: create a better block to cause a rollback and fail validation
		auto pBetterBlock = CreateValidBlockForDispatcherTests(keyPair);
		pBetterBlock->Timestamp = pBetterBlock->Timestamp - Timestamp(1000);
		context.setStatefulBlockValidationResult(ValidationResult::Failure);

		factory(test::CreateEntityRange({ pBetterBlock.get() }));
		WAIT_FOR_VALUE_EXPR(2u, context.numStatefulBlockValidatorCalls());

		// - failure is unknown until next rollback, alter timestamp not to hit recency cache
		pBetterBlock->Timestamp = pBetterBlock->Timestamp - Timestamp(2000);
		context.setStatefulBlockValidationResult(ValidationResult::Success);

		factory(test::CreateEntityRange({ pBetterBlock.get() }));
		WAIT_FOR_VALUE_EXPR(2u, context.numNewBlockSinkCalls());

		// Assert:
		EXPECT_EQ(3u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));

		EXPECT_EQ(1u, context.counter(Rollback_Elements_Committed_All));
		EXPECT_EQ(1u, context.counter(Rollback_Elements_Committed_Recent));
		EXPECT_EQ(1u, context.counter(Rollback_Elements_Ignored_All));
		EXPECT_EQ(1u, context.counter(Rollback_Elements_Ignored_Recent));
	}

	// endregion

	// region reputation

	namespace {
		template<typename TAssert>
		void AssertConsumeHandlesReputation(validators::ValidationResult validationResult, TAssert assertFunc) {
			// Arrange:
			TestContext context;
			context.setBlockValidationResults(ValidationResults(validationResult, ValidationResult::Success));
			context.boot();

			auto nodeIdentity = test::GenerateRandomByteArray<Key>();
			auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
			auto range = test::CreateEntityRange({ pNextBlock.get() });
			auto annotatedRange = model::AnnotatedBlockRange(std::move(range), nodeIdentity);

			{
				auto nodesModifier = context.testState().state().nodes().modifier();
				nodesModifier.add(ionet::Node(nodeIdentity, ionet::NodeEndpoint(), ionet::NodeMetadata()), ionet::NodeSource::Dynamic);
			}

			auto factory = context.testState().state().hooks().completionAwareBlockRangeConsumerFactory()(disruptor::InputSource::Local);

			// Act:
			std::atomic<size_t> numCompletions(0);
			factory(std::move(annotatedRange), [&numCompletions](auto, auto) { ++numCompletions; });
			WAIT_FOR_ONE(numCompletions);

			// - wait a bit to give the service time to consume more if there is a bug in the implementation
			test::Pause();

			// Assert:
			EXPECT_EQ(1u, context.counter(Block_Elements_Counter_Name));
			EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));

			auto interactions = context.testState().state().nodes().view().getNodeInfo(nodeIdentity).interactions(Timestamp());
			assertFunc(interactions);
		}
	}

	TEST(TEST_CLASS, Dispatcher_InteractionsAreUpdatedWhenValidationSucceeded) {
		AssertConsumeHandlesReputation(validators::ValidationResult::Success, [](const auto& interactions) {
			// Assert: interactions were updated
			EXPECT_EQ(1u, interactions.NumSuccesses);
			EXPECT_EQ(0u, interactions.NumFailures);
		});
	}

	TEST(TEST_CLASS, Dispatcher_InteractionsAreUpdatedWhenValidationFailed) {
		AssertConsumeHandlesReputation(validators::ValidationResult::Failure, [](const auto& interactions) {
			// Assert: interactions were updated
			EXPECT_EQ(0u, interactions.NumSuccesses);
			EXPECT_EQ(1u, interactions.NumFailures);
		});
	}

	TEST(TEST_CLASS, Dispatcher_InteractionsAreNotUpdatedWhenValidationIsNeutral) {
		AssertConsumeHandlesReputation(validators::ValidationResult::Neutral, [](const auto& interactions) {
			// Assert: interactions were not updated
			EXPECT_EQ(0u, interactions.NumSuccesses);
			EXPECT_EQ(0u, interactions.NumFailures);
		});
	}

	// endregion

	// region consume - transaction range

	namespace {
		template<typename THandler>
		void AssertCanConsumeTransactionRange(
				const ValidationResults& transactionValidationResults,
				model::AnnotatedTransactionRange&& range,
				THandler handler) {
			// Arrange:
			TestContext context;
			context.setTransactionValidationResults(transactionValidationResults);
			context.boot();
			auto factory = context.testState().state().hooks().transactionRangeConsumerFactory()(disruptor::InputSource::Local);

			// Act:
			factory(std::move(range));
			context.testState().state().tasks()[0].Callback(); // forward all batched transactions to the dispatcher
			WAIT_FOR_ONE_EXPR(context.counter(Transaction_Elements_Counter_Name));

			// - wait a bit to give the service time to consume more if there is a bug in the implementation
			test::Pause();

			// Assert:
			EXPECT_EQ(0u, context.counter(Block_Elements_Counter_Name));
			EXPECT_EQ(1u, context.counter(Transaction_Elements_Counter_Name));
			handler(context);
		}
	}

	TEST(TEST_CLASS, CanConsumeTransactionRange_InvalidElement_Stateless) {
		// Assert:
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateless = ValidationResult::Failure;
		AssertCanConsumeTransactionRange(transactionValidationResults, test::CreateTransactionEntityRange(1), [](const auto& context) {
			WAIT_FOR_ONE_EXPR(context.numTransactionStatuses());

			// - the transaction was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());
			EXPECT_EQ(1u, context.numTransactionStatuses());
		});
	}

	TEST(TEST_CLASS, CanConsumeTransactionRange_InvalidElement_Stateful) {
		// Assert:
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateful = ValidationResult::Failure;
		AssertCanConsumeTransactionRange(transactionValidationResults, test::CreateTransactionEntityRange(1), [](const auto& context) {
			WAIT_FOR_ONE_EXPR(context.numNewTransactionsSinkCalls());
			WAIT_FOR_ONE_EXPR(context.numTransactionStatuses());

			// - the transaction was forwarded to the sink (it could theoretically pass validation on another node)
			EXPECT_EQ(0u, context.numNewBlockSinkCalls());
			EXPECT_EQ(1u, context.numNewTransactionsSinkCalls());
			EXPECT_EQ(1u, context.numTransactionStatuses());
		});
	}

	TEST(TEST_CLASS, CanConsumeTransactionRange_ValidElement) {
		// Arrange: ensure deadline is in range
		auto signer = test::GenerateKeyPair();
		auto pValidTransaction = test::GenerateRandomTransaction();
		pValidTransaction->Signer = signer.publicKey();
		pValidTransaction->Deadline = utils::NetworkTime() + Timestamp(60'000);

		auto range = test::CreateEntityRange({ pValidTransaction.get() });

		// Assert:
		AssertCanConsumeTransactionRange(ValidationResults(), std::move(range), [](const auto& context) {
			WAIT_FOR_ONE_EXPR(context.numNewTransactionsSinkCalls());

			// - the transaction was forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockSinkCalls());
			EXPECT_EQ(1u, context.numNewTransactionsSinkCalls());
			EXPECT_EQ(0u, context.numTransactionStatuses());
		});
	}

	// endregion

	// region transaction status subscriber flush

	TEST(TEST_CLASS, BlockDispatcherFlushesTransactionStatusSubscriber) {
		// Arrange: notice that flush should be called even (especially) if block failed validation
		AssertCanConsumeBlockRange(test::CreateBlockEntityRange(1), [](const auto& context) {
			const auto& subscriber = context.testState().transactionStatusSubscriber();
			WAIT_FOR_ONE_EXPR(subscriber.numFlushes());

			// Assert: the subscriber was flushed
			EXPECT_EQ(1u, subscriber.numFlushes());
		});
	}

	TEST(TEST_CLASS, TransactionDispatcherFlushesTransactionStatusSubscriber) {
		// Arrange: notice that flush should be called even (especially) if transaction failed validation
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateless = ValidationResult::Failure;
		AssertCanConsumeTransactionRange(transactionValidationResults, test::CreateTransactionEntityRange(1), [](const auto& context) {
			const auto& subscriber = context.testState().transactionStatusSubscriber();
			WAIT_FOR_ONE_EXPR(subscriber.numFlushes());

			// Assert: the subscriber was flushed
			EXPECT_EQ(1u, subscriber.numFlushes());
		});
	}

	// endregion

	// region spam filtering

	namespace {
		void AssertTransactionSpamThrottleBehavior(bool enableFiltering, uint32_t maxCacheSize, uint32_t expectedCacheSize) {
			// Arrange:
			TestContext context;

			// - configure spam filter
			const auto& config = context.testState().config();
			auto& nodeConfig = const_cast<config::NodeConfiguration&>(config.Node);
			nodeConfig.ShouldEnableTransactionSpamThrottling = enableFiltering;
			nodeConfig.TransactionSpamThrottlingMaxBoostFee = Amount(10'000'000);
			nodeConfig.UnconfirmedTransactionsCacheMaxSize = maxCacheSize;
			const_cast<uint32_t&>(config.BlockChain.MaxTransactionsPerBlock) = maxCacheSize / 2;

			// - boot the service
			context.boot();
			auto factory = context.testState().state().hooks().transactionRangeConsumerFactory()(disruptor::InputSource::Local);

			// Act: try to fill the ut cache with transactions
			factory(test::CreateTransactionEntityRange(maxCacheSize));
			context.testState().state().tasks()[0].Callback(); // forward all batched transactions to the dispatcher

			// - wait for the transactions to flow through the consumers
			WAIT_FOR_ONE_EXPR(context.counter(Transaction_Elements_Counter_Name));
			WAIT_FOR_VALUE_EXPR(expectedCacheSize, context.testState().state().utCache().view().size());

			// Assert:
			EXPECT_EQ(expectedCacheSize, context.testState().state().utCache().view().size());
			EXPECT_EQ(maxCacheSize - expectedCacheSize, context.numTransactionStatuses());
		}
	}

	TEST(TEST_CLASS, CanDisableSpamThrottling) {
		// Assert: the entire cache should be filled
		AssertTransactionSpamThrottleBehavior(false, 50, 50);
	}

	TEST(TEST_CLASS, CanEnableSpamThrottling) {
		// Assert: the entire cache should not be filled because unimportant accounts are used
		AssertTransactionSpamThrottleBehavior(true, 50, 40);
	}

	// endregion
}}
