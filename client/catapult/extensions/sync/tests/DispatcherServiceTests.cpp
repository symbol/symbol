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
#include "catapult/cache_core/BlockStatisticCache.h"
#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/io/IndexFile.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/plugins/PluginLoader.h"
#include "catapult/preprocessor.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/Nemesis.h"
#include "tests/test/nodeps/TimeSupplier.h"
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

		crypto::KeyPair GetBlockSignerVrfKeyPair() {
			// random key is fine because signer vrf is configured in InitializeCatapultCacheForDispatcherTests
			return crypto::KeyPair::FromString("1A8BF1F961C6EB875D7C909E314DDA6D43AFAC182745D449B0671B8C455806C3");
		}

		cache::CatapultCache CreateCatapultCacheForDispatcherTests() {
			// importance grouping must be non-zero
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.ImportanceGrouping = 1;
			config.VotingSetGrouping = 1;

			// create the cache
			return test::CreateEmptyCatapultCache<test::CoreSystemCacheFactory>(config);
		}

		void InitializeCatapultCacheForDispatcherTests(
				cache::CatapultCache& cache,
				const crypto::KeyPair& signer,
				const Key& vrfPublicKey) {
			// create the delta
			auto delta = cache.createDelta();

			// set a difficulty for the nemesis block
			delta.sub<cache::BlockStatisticCache>().insert(state::BlockStatistic(Height(1)));

			// add a balance and importance for the signer
			auto& accountStateCache = delta.sub<cache::AccountStateCache>();
			accountStateCache.addAccount(signer.publicKey(), Height(1));
			auto& accountState = accountStateCache.find(signer.publicKey()).get();
			accountState.ImportanceSnapshots.set(Importance(1'000'000'000), model::ImportanceHeight(1));
			accountState.SupplementalPublicKeys.vrf().set(vrfPublicKey);

			// commit all changes
			cache.commit(Height(1));
		}

		std::shared_ptr<model::Block> CreateValidBlockForDispatcherTests(const crypto::KeyPair& signer) {
			constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;

			mocks::MockMemoryBlockStorage storage;
			auto pNemesisBlockElement = storage.loadBlockElement(Height(1));

			model::PreviousBlockContext context(*pNemesisBlockElement);
			auto pBlock = model::CreateBlock(context, Network_Identifier, signer.publicKey(), model::Transactions());
			pBlock->Timestamp = context.Timestamp + Timestamp(60000);

			auto vrfKeyPair = GetBlockSignerVrfKeyPair();
			auto vrfProof = crypto::GenerateVrfProof(context.GenerationHash, vrfKeyPair);
			pBlock->GenerationHashProof = { vrfProof.Gamma, vrfProof.VerificationHash, vrfProof.Scalar };

			model::SignBlockHeader(signer, *pBlock);
			return PORTABLE_MOVE(pBlock);
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

			ValidationResults(ValidationResult statelessResult, ValidationResult statefulResult)
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
			TestContext() : TestContext(test::CreateDefaultNetworkTimeSupplier())
			{}

			explicit TestContext(const supplier<Timestamp>& timeSupplier)
					: BaseType(CreateCatapultCacheForDispatcherTests(), timeSupplier)
					, m_numNewBlockSinkCalls(0)
					, m_numNewTransactionsSinkCalls(0)
					, m_pStatefulBlockValidator(nullptr) {
				// override data directory
				auto& state = testState().state();
				const_cast<std::string&>(state.config().User.DataDirectory) = m_tempDir.name();

				// initialize the cache
				InitializeCatapultCacheForDispatcherTests(state.cache(), GetBlockSignerKeyPair(), GetBlockSignerVrfKeyPair().publicKey());

				// set up sinks
				state.hooks().addNewBlockSink([&counter = m_numNewBlockSinkCalls](const auto&) { ++counter; });
				state.hooks().addNewTransactionsSink([&counter = m_numNewTransactionsSinkCalls](const auto&) { ++counter; });

				// configure subscribers
				testState().nodeSubscriber().enableBanSimulation();

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
						: std::make_pair(static_cast<consumers::CommitOperationStep>(std::numeric_limits<uint16_t>::max()), false);
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
				return m_pStatefulBlockValidator->numNotificationTypes();
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

		public:
			size_t bannedNodesSize() {
				return testState().state().nodes().view().bannedNodesSize();
			}

			size_t bannedNodesDeepSize() {
				return testState().state().nodes().view().bannedNodesDeepSize();
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
		EXPECT_EQ(7u, blockDispatcherStatus.Size);
		EXPECT_TRUE(blockDispatcherStatus.IsRunning);

		// - transaction dispatcher should be initialized
		auto transactionDispatcherStatus = GetTransactionDispatcherStatus(context.locator());
		EXPECT_EQ("transaction dispatcher", transactionDispatcherStatus.Name);
		EXPECT_EQ(5u, transactionDispatcherStatus.Size);
		EXPECT_TRUE(transactionDispatcherStatus.IsRunning);
	}

	TEST(TEST_CLASS, CanBootServiceWithAuditingEnabled) {
		// Arrange: enable auditing
		TestContext context;
		const_cast<bool&>(context.testState().config().Node.EnableDispatcherInputAuditing) = true;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(Num_Expected_Services, context.locator().numServices());
		EXPECT_EQ(Num_Expected_Counters, context.locator().counters().size());
		EXPECT_EQ(Num_Expected_Tasks, context.testState().state().tasks().size());

		EXPECT_EQ(8u, GetBlockDispatcherStatus(context.locator()).Size);
		EXPECT_EQ(6u, GetTransactionDispatcherStatus(context.locator()).Size);

		// - auditing directories were created
		auto auditDirectory = context.tempPath() / "audit";
		EXPECT_TRUE(boost::filesystem::is_directory(auditDirectory / "block dispatcher"));
		EXPECT_TRUE(boost::filesystem::is_directory(auditDirectory / "transaction dispatcher"));
	}

	TEST(TEST_CLASS, CanBootServiceWithAutoSyncCleanupEnabled) {
		// Arrange: enable cleanup
		TestContext context;
		const_cast<bool&>(context.testState().config().Node.EnableAutoSyncCleanup) = true;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(Num_Expected_Services, context.locator().numServices());
		EXPECT_EQ(Num_Expected_Counters, context.locator().counters().size());
		EXPECT_EQ(Num_Expected_Tasks, context.testState().state().tasks().size());

		EXPECT_EQ(8u, GetBlockDispatcherStatus(context.locator()).Size);
		EXPECT_EQ(5u, GetTransactionDispatcherStatus(context.locator()).Size);
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

	TEST(TEST_CLASS, TasksAreRegistered) {
		test::AssertRegisteredTasks(TestContext(), { "batch transaction task" });
	}

	// endregion

	// region consume - block range

	namespace {
		template<typename THandler>
		void AssertCanConsumeBlockRange(bool enableVerifiableReceipts, model::AnnotatedBlockRange&& range, THandler handler) {
			// Arrange:
			TestContext context;
			const auto& blockChainConfig = context.testState().config().BlockChain;
			const_cast<model::BlockChainConfiguration&>(blockChainConfig).EnableVerifiableReceipts = enableVerifiableReceipts;

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

	TEST(TEST_CLASS, CanConsumeBlockRange_InvalidElement_Unsigned) {
		// Arrange:
		auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		pNextBlock->Signature[0] ^= 0xFF;
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// Act + Assert:
		AssertCanConsumeBlockRange(std::move(range), AssertBlockRangeInvalidElement);
	}

	TEST(TEST_CLASS, CanConsumeBlockRange_InvalidElement_Signed) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		auto pNextBlock = test::GenerateEmptyRandomBlock();
		pNextBlock->SignerPublicKey = keyPair.publicKey();
		model::SignBlockHeader(keyPair, *pNextBlock);
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// Act + Assert:
		AssertCanConsumeBlockRange(std::move(range), AssertBlockRangeInvalidElement);
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
			block.ReceiptsHash = model::CalculateMerkleHash(*blockStatementBuilder.build());
		}
	}

	TEST(TEST_CLASS, CanConsumeBlockRange_ValidElement_WithVerifiableReceipts) {
		// Arrange: block contains correct block receipts hash, so should get accepted
		auto signer = GetBlockSignerKeyPair();
		auto pNextBlock = CreateValidBlockForDispatcherTests(signer);
		SetBlockReceiptsHash(*pNextBlock);
		model::SignBlockHeader(signer, *pNextBlock);
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
		// Assert: TransactionsHash mismatch
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
		model::SignBlockHeader(keyPair, *pBetterBlock);

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
		model::SignBlockHeader(keyPair, *pBetterBlock);

		context.setStatefulBlockValidationResult(ValidationResult::Failure);

		factory(test::CreateEntityRange({ pBetterBlock.get() }));
		WAIT_FOR_VALUE_EXPR(2u, context.numStatefulBlockValidatorCalls());

		// - failure is unknown until next rollback, alter timestamp not to hit recency cache
		pBetterBlock->Timestamp = pBetterBlock->Timestamp - Timestamp(2000);
		model::SignBlockHeader(keyPair, *pBetterBlock);

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

			auto nodeIdentity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
			auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
			auto range = test::CreateEntityRange({ pNextBlock.get() });
			auto annotatedRange = model::AnnotatedBlockRange(std::move(range), nodeIdentity);

			{
				auto nodesModifier = context.testState().state().nodes().modifier();
				nodesModifier.add(ionet::Node(nodeIdentity), ionet::NodeSource::Dynamic);
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

	// region banning - block range consumer

	namespace {
		struct CompletionAwareTraits {
			static auto CreateFactory(TestContext& context, disruptor::InputSource inputSource) {
				return context.testState().state().hooks().completionAwareBlockRangeConsumerFactory()(inputSource);
			}

			static auto ConsumeRange(
					const chain::CompletionAwareBlockRangeConsumerFunc& factory,
					model::AnnotatedBlockRange&& annotatedRange) {
				return factory(std::move(annotatedRange), [](auto, auto) {});
			}
		};

		struct CompletionUnawareTraits {
			static auto CreateFactory(TestContext& context, disruptor::InputSource inputSource) {
				return context.testState().state().hooks().blockRangeConsumerFactory()(inputSource);
			}

			static auto ConsumeRange(const extensions::BlockRangeConsumerFunc& factory, model::AnnotatedBlockRange&& annotatedRange) {
				factory(std::move(annotatedRange));
				return 0u;
			}
		};
	}

#define CONSUMER_FACTORY_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_CompletionAware) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CompletionAwareTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_CompletionUnaware) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<CompletionUnawareTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		supplier<Timestamp> CreateTimeSupplier(const std::vector<uint32_t>& rawTimestamps) {
			return test::CreateTimeSupplierFromMilliseconds(rawTimestamps, 1000 * 60 * 60);
		}

		template<typename TTraits>
		void AssertBlockRangeConsumeHandlesBanning(validators::ValidationResult validationResult, size_t numExpectedBannedAccounts) {
			// Arrange:
			TestContext context;
			context.setBlockValidationResults(ValidationResults(validationResult, ValidationResult::Success));
			context.boot();

			auto nodeIdentity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
			auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
			auto annotatedRange = model::AnnotatedBlockRange(test::CreateEntityRange({ pNextBlock.get() }), nodeIdentity);

			auto factory = TTraits::CreateFactory(context, disruptor::InputSource::Local);

			// Act:
			TTraits::ConsumeRange(factory, std::move(annotatedRange));

			WAIT_FOR_ONE_EXPR(context.counter(Block_Elements_Counter_Name));
			WAIT_FOR_ZERO_EXPR(context.counter(Block_Elements_Active_Counter_Name));

			// - wait a bit to give the service time to consume more if there is a bug in the implementation
			test::Pause();

			// Assert:
			EXPECT_EQ(1u, context.counter(Block_Elements_Counter_Name));
			EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));
			EXPECT_EQ(numExpectedBannedAccounts, context.bannedNodesSize());
			EXPECT_EQ(numExpectedBannedAccounts, context.bannedNodesDeepSize());
		}

		template<typename TTraits>
		void AssertBlockDispatcherHandlesBannedNode(
				disruptor::InputSource inputSource,
				size_t numExpectedCompletions,
				size_t numExpectedBannedAccounts) {
			// Arrange:
			TestContext context;
			context.setBlockValidationResults(ValidationResults(validators::ValidationResult::Failure, ValidationResult::Success));
			context.boot();

			auto nodeIdentity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
			auto pNextBlock1 = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
			auto annotatedRange1 = model::AnnotatedBlockRange(test::CreateEntityRange({ pNextBlock1.get() }), nodeIdentity);

			auto factory = TTraits::CreateFactory(context, inputSource);
			TTraits::ConsumeRange(factory, std::move(annotatedRange1));

			WAIT_FOR_ONE_EXPR(context.counter(Block_Elements_Counter_Name));
			WAIT_FOR_ZERO_EXPR(context.counter(Block_Elements_Active_Counter_Name));

			// Sanity:
			EXPECT_EQ(1u, context.counter(Block_Elements_Counter_Name));
			EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));
			EXPECT_EQ(1u, context.bannedNodesSize());
			EXPECT_EQ(1u, context.bannedNodesDeepSize());

			// Act: feed another range
			auto pNextBlock2 = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
			auto annotatedRange2 = model::AnnotatedBlockRange(test::CreateEntityRange({ pNextBlock2.get() }), nodeIdentity);

			auto id = TTraits::ConsumeRange(factory, std::move(annotatedRange2));

			WAIT_FOR_VALUE_EXPR(numExpectedCompletions, context.counter(Block_Elements_Counter_Name));
			WAIT_FOR_ZERO_EXPR(context.counter(Block_Elements_Active_Counter_Name));

			// - wait a bit to give the service time to consume more if there is a bug in the implementation
			test::Pause();

			if (disruptor::InputSource::Local == inputSource)
				EXPECT_NE(0u, id);
			else
				EXPECT_EQ(0u, id);

			EXPECT_EQ(numExpectedCompletions, context.counter(Block_Elements_Counter_Name));
			EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));
			EXPECT_EQ(numExpectedBannedAccounts, context.bannedNodesSize());
			EXPECT_EQ(numExpectedBannedAccounts, context.bannedNodesDeepSize());
		}
	}

	CONSUMER_FACTORY_TRAITS_BASED_TEST(Dispatcher_NodeIsNotBannedWhenStatelessValidationIsSuccess_BlockRange) {
		AssertBlockRangeConsumeHandlesBanning<TTraits>(validators::ValidationResult::Success, 0);
	}

	CONSUMER_FACTORY_TRAITS_BASED_TEST(Dispatcher_NodeIsBannedWhenStatelessValidationIsNeutral_BlockRange) {
		// note that this will not happen in a real scenario because there is no neutral result for stateless validation
		AssertBlockRangeConsumeHandlesBanning<TTraits>(validators::ValidationResult::Neutral, 1);
	}

	CONSUMER_FACTORY_TRAITS_BASED_TEST(Dispatcher_NodeIsBannedWhenStatelessValidationIsFailure_BlockRange) {
		AssertBlockRangeConsumeHandlesBanning<TTraits>(validators::ValidationResult::Failure, 1);
	}

	CONSUMER_FACTORY_TRAITS_BASED_TEST(Dispatcher_BlockRangeFromBannedRemoteNodeIsIgnored) {
		AssertBlockDispatcherHandlesBannedNode<TTraits>(disruptor::InputSource::Remote_Push, 1, 1);
	}

	TEST(TEST_CLASS, Dispatcher_BlockRangeFromBannedLocalNodeIsProcessed) {
		AssertBlockDispatcherHandlesBannedNode<CompletionAwareTraits>(disruptor::InputSource::Local, 2, 1);
	}

	CONSUMER_FACTORY_TRAITS_BASED_TEST(Dispatcher_NodeIsNotBannedWhenStatefulValidationIsFailure_BlockRange) {
		// Arrange:
		TestContext context;
		context.setBlockValidationResults(ValidationResults(ValidationResult::Success, ValidationResult::Failure));
		context.boot();

		auto nodeIdentity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		auto annotatedRange = model::AnnotatedBlockRange(test::CreateEntityRange({ pNextBlock.get() }), nodeIdentity);

		auto factory = TTraits::CreateFactory(context, disruptor::InputSource::Local);

		// Act:
		TTraits::ConsumeRange(factory, std::move(annotatedRange));

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Assert:
		EXPECT_EQ(1u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));
		EXPECT_EQ(0u, context.bannedNodesSize());
		EXPECT_EQ(0u, context.bannedNodesDeepSize());
	}

	CONSUMER_FACTORY_TRAITS_BASED_TEST(Dispatcher_NodeIsNotBannedWhenHostIsInLocalNetworks_BlockRange) {
		// Arrange:
		TestContext context;
		const_cast<config::NodeConfiguration&>(context.testState().state().config().Node).LocalNetworks.emplace("123.456.789");
		context.setBlockValidationResults(ValidationResults(ValidationResult::Failure, ValidationResult::Success));
		context.boot();

		auto nodeIdentity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "123.456.789.123" };
		auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		auto annotatedRange = model::AnnotatedBlockRange(test::CreateEntityRange({ pNextBlock.get() }), nodeIdentity);

		auto factory = TTraits::CreateFactory(context, disruptor::InputSource::Local);

		// Act:
		TTraits::ConsumeRange(factory, std::move(annotatedRange));

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Assert:
		EXPECT_EQ(1u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));
		EXPECT_EQ(0u, context.bannedNodesSize());
		EXPECT_EQ(0u, context.bannedNodesDeepSize());
	}

	CONSUMER_FACTORY_TRAITS_BASED_TEST(Dispatcher_BannedNodesArePruned) {
		// Arrange:
		TestContext context(CreateTimeSupplier({ 1, 1, 1, 5 }));
		context.setBlockValidationResults(ValidationResults(ValidationResult::Failure, ValidationResult::Success));
		context.boot();

		auto nodeIdentity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto pNextBlock1 = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		auto annotatedRange1 = model::AnnotatedBlockRange(test::CreateEntityRange({ pNextBlock1.get() }), nodeIdentity);

		auto factory = TTraits::CreateFactory(context, disruptor::InputSource::Local);

		// Act:
		TTraits::ConsumeRange(factory, std::move(annotatedRange1));

		WAIT_FOR_ONE_EXPR(context.counter(Block_Elements_Counter_Name));
		WAIT_FOR_ZERO_EXPR(context.counter(Block_Elements_Active_Counter_Name));

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Sanity: identity is banned at 1h
		EXPECT_EQ(1u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(0u, context.counter(Transaction_Elements_Counter_Name));
		EXPECT_EQ(1u, context.bannedNodesSize());
		EXPECT_EQ(1u, context.bannedNodesDeepSize());

		// Act: feed another range to invoke prune at 5h
		auto pNextBlock2 = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		auto annotatedRange2 = model::AnnotatedBlockRange(test::CreateEntityRange({ pNextBlock2.get() }), nodeIdentity);

		TTraits::ConsumeRange(factory, std::move(annotatedRange2));

		WAIT_FOR_VALUE_EXPR(2u, context.counter(Block_Elements_Counter_Name));
		WAIT_FOR_ZERO_EXPR(context.counter(Block_Elements_Active_Counter_Name));

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Assert: node was pruned
		EXPECT_EQ(0u, context.bannedNodesSize());
		EXPECT_EQ(0u, context.bannedNodesDeepSize());
	}

	// endregion

	// region consume - transaction range

	namespace {
		model::TransactionRange CreateSignedTransactionEntityRange(size_t numTransactions) {
			auto range = test::CreateTransactionEntityRange(numTransactions);
			for (auto& transaction : range) {
				auto keyPair = test::GenerateKeyPair();
				transaction.SignerPublicKey = keyPair.publicKey();
				transaction.Deadline = test::CreateDefaultNetworkTimeSupplier()() + Timestamp(60'000);
				extensions::TransactionExtensions(test::GetNemesisGenerationHashSeed()).sign(keyPair, transaction);
			}

			return range;
		}

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

		void AssertTransactionRangeConsumeHandlesBanning(
				const ValidationResults& transactionValidationResults,
				model::AnnotatedTransactionRange&& range,
				size_t numExpectedBannedAccounts) {
			// Arrange:
			TestContext context;
			context.setTransactionValidationResults(transactionValidationResults);
			context.boot();
			auto factory = context.testState().state().hooks().transactionRangeConsumerFactory()(disruptor::InputSource::Local);

			// Act:
			factory(std::move(range));
			context.testState().state().tasks()[0].Callback(); // forward all batched transactions to the dispatcher
			WAIT_FOR_ONE_EXPR(context.counter(Transaction_Elements_Counter_Name));
			WAIT_FOR_ZERO_EXPR(context.counter(Transaction_Elements_Active_Counter_Name));

			// - wait a bit to give the service time to consume more if there is a bug in the implementation
			test::Pause();

			// Assert:
			EXPECT_EQ(0u, context.counter(Block_Elements_Counter_Name));
			EXPECT_EQ(1u, context.counter(Transaction_Elements_Counter_Name));
			EXPECT_EQ(numExpectedBannedAccounts, context.bannedNodesSize());
			EXPECT_EQ(numExpectedBannedAccounts, context.bannedNodesDeepSize());
		}
	}

	TEST(TEST_CLASS, CanConsumeTransactionRange_InvalidElement_Unsigned) {
		// Arrange:
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateless = ValidationResult::Failure;
		AssertCanConsumeTransactionRange(transactionValidationResults, test::CreateTransactionEntityRange(1), [](const auto& context) {
			WAIT_FOR_ONE_EXPR(context.numTransactionStatuses());

			// Assert: the transaction was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());
			EXPECT_EQ(1u, context.numTransactionStatuses());
		});
	}

	TEST(TEST_CLASS, CanConsumeTransactionRange_InvalidElement_Stateless) {
		// Arrange:
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateless = ValidationResult::Failure;
		AssertCanConsumeTransactionRange(transactionValidationResults, CreateSignedTransactionEntityRange(1), [](const auto& context) {
			WAIT_FOR_ONE_EXPR(context.numTransactionStatuses());

			// Assert: the transaction was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());
			EXPECT_EQ(1u, context.numTransactionStatuses());
		});
	}

	TEST(TEST_CLASS, CanConsumeTransactionRange_InvalidElement_Stateful) {
		// Arrange:
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateful = ValidationResult::Failure;
		AssertCanConsumeTransactionRange(transactionValidationResults, CreateSignedTransactionEntityRange(1), [](const auto& context) {
			WAIT_FOR_ONE_EXPR(context.numNewTransactionsSinkCalls());
			WAIT_FOR_ONE_EXPR(context.numTransactionStatuses());

			// Assert: the transaction was forwarded to the sink (it could theoretically pass validation on another node)
			EXPECT_EQ(0u, context.numNewBlockSinkCalls());
			EXPECT_EQ(1u, context.numNewTransactionsSinkCalls());
			EXPECT_EQ(1u, context.numTransactionStatuses());
		});
	}

	TEST(TEST_CLASS, CanConsumeTransactionRange_ValidElement) {
		// Arrange: ensure deadline is in range
		auto signer = test::GenerateKeyPair();
		auto pValidTransaction = test::GenerateRandomTransaction();
		pValidTransaction->SignerPublicKey = signer.publicKey();
		pValidTransaction->Deadline = test::CreateDefaultNetworkTimeSupplier()() + Timestamp(60'000);
		extensions::TransactionExtensions(test::GetNemesisGenerationHashSeed()).sign(signer, *pValidTransaction);

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

	// region banning - transaction range consumer

	TEST(TEST_CLASS, Dispatcher_NodeIsNotBannedWhenStatelessValidationIsSuccess_TransactionRange) {
		AssertTransactionRangeConsumeHandlesBanning(ValidationResults(), CreateSignedTransactionEntityRange(1), 0);
	}

	TEST(TEST_CLASS, Dispatcher_NodeIsBannedWhenStatelessValidationIsNeutral_TransactionRange) {
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateless = ValidationResult::Neutral;
		auto range = CreateSignedTransactionEntityRange(1);
		AssertTransactionRangeConsumeHandlesBanning(transactionValidationResults, std::move(range), 1);
	}

	TEST(TEST_CLASS, Dispatcher_NodeIsBannedWhenStatelessValidationIsFailure_TransactionRange) {
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateless = ValidationResult::Failure;
		auto range = test::CreateTransactionEntityRange(1);
		AssertTransactionRangeConsumeHandlesBanning(transactionValidationResults, std::move(range), 1);
	}

	TEST(TEST_CLASS, Dispatcher_NodeIsNotBannedWhenStatefulValidationIsFailure_TransactionRange) {
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateful = ValidationResult::Failure;
		AssertTransactionRangeConsumeHandlesBanning(transactionValidationResults, CreateSignedTransactionEntityRange(1), 0);
	}

	TEST(TEST_CLASS, Dispatcher_NodeIsNotBannedWhenHostIsInLocalNetworks_TransactionRange) {
		// Arrange:
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateless = ValidationResult::Failure;
		TestContext context;
		const_cast<config::NodeConfiguration&>(context.testState().state().config().Node).LocalNetworks.emplace("123.456.789");
		context.setTransactionValidationResults(transactionValidationResults);
		context.boot();
		auto factory = context.testState().state().hooks().transactionRangeConsumerFactory()(disruptor::InputSource::Local);

		// Act:
		auto nodeIdentity = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "123.456.789.123" };
		auto annotatedRange = model::AnnotatedTransactionRange(test::CreateTransactionEntityRange(1), nodeIdentity);
		factory(std::move(annotatedRange));
		context.testState().state().tasks()[0].Callback();
		WAIT_FOR_ONE_EXPR(context.counter(Transaction_Elements_Counter_Name));
		WAIT_FOR_ZERO_EXPR(context.counter(Transaction_Elements_Active_Counter_Name));

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Assert:
		EXPECT_EQ(0u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(1u, context.counter(Transaction_Elements_Counter_Name));
		EXPECT_EQ(0u, context.bannedNodesSize());
		EXPECT_EQ(0u, context.bannedNodesDeepSize());
	}

	TEST(TEST_CLASS, Dispatcher_TransactionRangeFromBannedNodeIsIgnored) {
		// Arrange:
		auto signer = test::GenerateKeyPair();
		auto pTransaction = test::GenerateRandomTransaction();
		pTransaction->SignerPublicKey = signer.publicKey();

		auto nodeIdentity = model::NodeIdentity{ pTransaction->SignerPublicKey, "11.22.33.44" };
		auto range = model::AnnotatedTransactionRange(test::CreateEntityRange({ pTransaction.get() }), nodeIdentity);

		TestContext context;
		context.setTransactionValidationResults(ValidationResults());
		context.boot();
		auto factory = context.testState().state().hooks().transactionRangeConsumerFactory()(disruptor::InputSource::Local);

		factory(std::move(range));
		context.testState().state().tasks()[0].Callback(); // forward all batched transactions to the dispatcher
		WAIT_FOR_ONE_EXPR(context.counter(Transaction_Elements_Counter_Name));
		WAIT_FOR_ZERO_EXPR(context.counter(Transaction_Elements_Active_Counter_Name));

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Sanity:
		EXPECT_EQ(0u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(1u, context.counter(Transaction_Elements_Counter_Name));
		EXPECT_EQ(1u, context.bannedNodesSize());
		EXPECT_EQ(1u, context.bannedNodesDeepSize());

		// Act: valid transaction range is not processed
		pTransaction->Deadline = test::CreateDefaultNetworkTimeSupplier()() + Timestamp(60'000);
		extensions::TransactionExtensions(test::GetNemesisGenerationHashSeed()).sign(signer, *pTransaction);

		auto range2 = model::AnnotatedTransactionRange(test::CreateEntityRange({ pTransaction.get() }), nodeIdentity);

		factory(std::move(range2));
		context.testState().state().tasks()[0].Callback(); // forward all batched transactions to the dispatcher

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Assert: transaction element was not consumed
		EXPECT_EQ(0u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(1u, context.counter(Transaction_Elements_Counter_Name));
		EXPECT_EQ(1u, context.bannedNodesSize());
		EXPECT_EQ(1u, context.bannedNodesDeepSize());
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
			nodeConfig.EnableTransactionSpamThrottling = enableFiltering;
			nodeConfig.TransactionSpamThrottlingMaxBoostFee = Amount(10'000'000);
			nodeConfig.UnconfirmedTransactionsCacheMaxSize = maxCacheSize;
			const_cast<uint32_t&>(config.BlockChain.MaxTransactionsPerBlock) = maxCacheSize / 2;

			// - boot the service
			context.boot();
			auto factory = context.testState().state().hooks().transactionRangeConsumerFactory()(disruptor::InputSource::Local);

			// Act: try to fill the ut cache with transactions
			factory(CreateSignedTransactionEntityRange(maxCacheSize));
			context.testState().state().tasks()[0].Callback(); // forward all batched transactions to the dispatcher

			// - wait for the transactions to flow through the consumers
			const auto& utCache = const_cast<const extensions::ServiceState&>(context.testState().state()).utCache();
			WAIT_FOR_ONE_EXPR(context.counter(Transaction_Elements_Counter_Name));
			WAIT_FOR_VALUE_EXPR(expectedCacheSize, utCache.view().size());

			// Assert:
			EXPECT_EQ(expectedCacheSize, utCache.view().size());
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

	// region ban counters

	TEST(TEST_CLASS, Dispatcher_BanCountersHaveExpectedValues) {
		// Arrange:
		ValidationResults transactionValidationResults;
		transactionValidationResults.Stateless = ValidationResult::Failure;
		TestContext context(CreateTimeSupplier({ 1, 1, 1, 4 }));
		context.setTransactionValidationResults(transactionValidationResults);
		context.boot();

		auto nodeIdentity1 = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto annotatedRange1 = model::AnnotatedTransactionRange(test::CreateTransactionEntityRange(1), nodeIdentity1);
		auto factory = context.testState().state().hooks().transactionRangeConsumerFactory()(disruptor::InputSource::Local);

		// Act: ban first node
		factory(std::move(annotatedRange1));
		context.testState().state().tasks()[0].Callback(); // forward all batched transactions to the dispatcher
		WAIT_FOR_ONE_EXPR(context.counter(Transaction_Elements_Counter_Name));
		WAIT_FOR_ZERO_EXPR(context.counter(Transaction_Elements_Active_Counter_Name));

		// - wait a bit to give the service time to consume more if there is a bug in the implementation
		test::Pause();

		// Sanity:
		EXPECT_EQ(0u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(1u, context.counter(Transaction_Elements_Counter_Name));
		EXPECT_EQ(1u, context.bannedNodesSize());
		EXPECT_EQ(1u, context.bannedNodesDeepSize());

		auto nodeIdentity2 = model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		auto annotatedRange2 = model::AnnotatedTransactionRange(test::CreateTransactionEntityRange(1), nodeIdentity2);

		// Act: ban a second node
		factory(std::move(annotatedRange2));
		context.testState().state().tasks()[0].Callback(); // forward all batched transactions to the dispatcher
		WAIT_FOR_VALUE_EXPR(2u, context.counter(Transaction_Elements_Counter_Name));
		WAIT_FOR_ZERO_EXPR(context.counter(Transaction_Elements_Active_Counter_Name));

		// Assert: first node was banned at 1h, ban expires at 2h, not eligible for pruning at 4h
		EXPECT_EQ(0u, context.counter(Block_Elements_Counter_Name));
		EXPECT_EQ(2u, context.counter(Transaction_Elements_Counter_Name));
		EXPECT_EQ(1u, context.bannedNodesSize());
		EXPECT_EQ(2u, context.bannedNodesDeepSize());
	}

	// endregion
}}
