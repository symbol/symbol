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
			auto& accountState = delta.sub<cache::AccountStateCache>().addAccount(signer.publicKey(), Height(1));
			accountState.Balances.credit(Xem_Id, Amount(1'000'000'000'000));
			accountState.ImportanceInfo.set(Importance(1'000'000'000), model::ImportanceHeight(1));

			// commit all changes
			cache.commit(Height(1));
		}

		std::shared_ptr<model::Block> CreateValidBlockForDispatcherTests(const crypto::KeyPair& signer) {
			constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

			mocks::MockMemoryBasedStorage storage;
			auto pNemesisBlockElement = storage.loadBlockElement(Height(1));

			model::PreviousBlockContext context(*pNemesisBlockElement);
			auto pBlock = model::CreateBlock(context, Network_Identifier, signer.publicKey(), model::Transactions());
			pBlock->Timestamp = context.Timestamp + Timestamp(60000);
			test::SignBlock(signer, *pBlock);
			return std::move(pBlock);
		}

		// endregion

		struct DispatcherServiceTraits {
			static constexpr auto CreateRegistrar = CreateDispatcherServiceRegistrar;
		};

		struct TransactionValidationResults {
		public:
			TransactionValidationResults()
					: Stateless(ValidationResult::Success)
					, Stateful(ValidationResult::Success)
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
					, m_numNewTransactionsSinkCalls(0) {
				// initialize the cache
				auto& state = testState().state();
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

				// add block validator to emulate block validation failure
				pluginManager.addStatefulValidatorHook([this](auto& builder) {
					auto pValidator = std::make_unique<mocks::MockNotificationValidatorT<model::BlockNotification>>();
					pValidator->setResult(ValidationResult::Success);
					m_pBlockValidator = pValidator.get();
					builder.template add<model::BlockNotification>(std::move(pValidator));
				});
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

			size_t numBlockValidatorCalls() const {
				return m_pBlockValidator->notificationTypes().size();
			}

		public:
			void setTransactionValidationResults(const TransactionValidationResults& transactionValidationResults) {
				m_transactionValidationResults = transactionValidationResults;
			}

			void setBlockValidationResult(validators::ValidationResult result) {
				m_pBlockValidator->setResult(result);
			}

		private:
			TransactionValidationResults m_transactionValidationResults;
			std::atomic<size_t> m_numNewBlockSinkCalls;
			std::atomic<size_t> m_numNewTransactionsSinkCalls;
			mocks::MockNotificationValidatorT<model::BlockNotification>* m_pBlockValidator;
		};

		// region DispatcherStatus

		struct DispatcherStatus {
			std::string Name;
			size_t Size;
			bool IsRunning;
		};

		DispatcherStatus GetDispatcherStatus(const std::shared_ptr<disruptor::ConsumerDispatcher>& pDispatcher) {
			return !pDispatcher
					? DispatcherStatus{}
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
		// Arrange:
		TestContext context;

		// - enable auditing
		test::TempDirectoryGuard tempDirectoryGuard;
		const auto& config = context.testState().config();
		const_cast<std::string&>(config.User.DataDirectory) = tempDirectoryGuard.name();
		const_cast<bool&>(config.Node.ShouldAuditDispatcherInputs) = true;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(Num_Expected_Services, context.locator().numServices());
		EXPECT_EQ(Num_Expected_Counters, context.locator().counters().size());
		EXPECT_EQ(Num_Expected_Tasks, context.testState().state().tasks().size());

		EXPECT_EQ(7u, GetBlockDispatcherStatus(context.locator()).Size);
		EXPECT_EQ(5u, GetTransactionDispatcherStatus(context.locator()).Size);

		// - auditing directories were created
		auto auditDirectory = boost::filesystem::path(tempDirectoryGuard.name()) / "audit";
		EXPECT_TRUE(boost::filesystem::is_directory(auditDirectory / "block dispatcher"));
		EXPECT_TRUE(boost::filesystem::is_directory(auditDirectory / "transaction dispatcher"));
	}

	TEST(TEST_CLASS, CanBootServiceWithAddressPrecomputationEnabled) {
		// Arrange:
		TestContext context;
		const auto& config = context.testState().config();
		const_cast<bool&>(config.Node.ShouldPrecomputeTransactionAddresses) = true;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(Num_Expected_Services + 1, context.locator().numServices());
		EXPECT_EQ(Num_Expected_Counters, context.locator().counters().size());
		EXPECT_EQ(Num_Expected_Tasks, context.testState().state().tasks().size());

		EXPECT_EQ(7u, GetBlockDispatcherStatus(context.locator()).Size);
		EXPECT_EQ(5u, GetTransactionDispatcherStatus(context.locator()).Size);

		// - notification publisher service should exist
		EXPECT_TRUE(!!context.locator().service<model::NotificationPublisher>("dispatcher.notificationPublisher"));
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

	// region consume

	namespace {
		template<typename THandler>
		void AssertCanConsumeBlockRange(model::BlockRange&& range, THandler handler) {
			// Arrange:
			TestContext context;
			context.boot();
			auto factory = context.testState().state().hooks().blockRangeConsumerFactory()(disruptor::InputSource::Local);

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
	}

	TEST(TEST_CLASS, CanConsumeBlockRange_InvalidElement) {
		// Assert:
		AssertCanConsumeBlockRange(test::CreateBlockEntityRange(1), [](const auto& context) {
			// - the block was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());
		});
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
		});
	}

	namespace {
		template<typename THandler>
		void AssertCanConsumeBlockRangeCompletionAware(model::BlockRange&& range, THandler handler) {
			// Arrange:
			TestContext context;
			context.boot();
			auto factory = context.testState().state().hooks().completionAwareBlockRangeConsumerFactory()(disruptor::InputSource::Local);

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
	}

	TEST(TEST_CLASS, CanConsumeBlockRangeCompletionAware_InvalidElement) {
		// Assert:
		AssertCanConsumeBlockRangeCompletionAware(test::CreateBlockEntityRange(1), [](const auto& context) {
			// - the block was not forwarded to the sink
			EXPECT_EQ(0u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());

			// - state change subscriber shouldn't have been called for an invalid element, so chain score should be unchanged (zero)
			const auto& stateChangeSubscriber = context.testState().stateChangeSubscriber();
			EXPECT_EQ(0u, stateChangeSubscriber.numScoreChanges());
			EXPECT_EQ(0u, stateChangeSubscriber.numStateChanges());
			EXPECT_EQ(model::ChainScore(), stateChangeSubscriber.lastChainScore());
		});
	}

	TEST(TEST_CLASS, CanConsumeBlockRangeCompletionAware_ValidElement) {
		// Arrange:
		auto pNextBlock = CreateValidBlockForDispatcherTests(GetBlockSignerKeyPair());
		auto range = test::CreateEntityRange({ pNextBlock.get() });

		// Assert:
		AssertCanConsumeBlockRangeCompletionAware(std::move(range), [](const auto& context) {
			// - the block was forwarded to the sink
			EXPECT_EQ(1u, context.numNewBlockSinkCalls());
			EXPECT_EQ(0u, context.numNewTransactionsSinkCalls());

			// - state change subscriber should have been called, so chain score should be changed (non-zero)
			const auto& stateChangeSubscriber = context.testState().stateChangeSubscriber();
			EXPECT_EQ(1u, stateChangeSubscriber.numScoreChanges());
			EXPECT_EQ(1u, stateChangeSubscriber.numStateChanges());
			EXPECT_EQ(model::ChainScore(99'999'999'999'940), stateChangeSubscriber.lastChainScore());
		});
	}

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
		test::SignBlock(GetBlockSignerKeyPair(), *pBetterBlock);

		factory(test::CreateEntityRange({ pBetterBlock.get() }));
		WAIT_FOR_ONE_EXPR(context.counter(Rollback_Elements_Committed_All));

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
		test::SignBlock(GetBlockSignerKeyPair(), *pBetterBlock);
		context.setBlockValidationResult(ValidationResult::Failure);

		factory(test::CreateEntityRange({ pBetterBlock.get() }));
		WAIT_FOR_VALUE_EXPR(2u, context.numBlockValidatorCalls());

		// - failure is unknown until next rollback, alter timestamp not to hit recency cache
		pBetterBlock->Timestamp = pBetterBlock->Timestamp - Timestamp(2000);
		test::SignBlock(GetBlockSignerKeyPair(), *pBetterBlock);
		context.setBlockValidationResult(ValidationResult::Success);

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

	namespace {
		template<typename THandler>
		void AssertCanConsumeTransactionRange(
				const TransactionValidationResults& transactionValidationResults,
				model::TransactionRange&& range,
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
		TransactionValidationResults transactionValidationResults;
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
		TransactionValidationResults transactionValidationResults;
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
		extensions::SignTransaction(signer, *pValidTransaction);

		auto range = test::CreateEntityRange({ pValidTransaction.get() });

		// Assert:
		AssertCanConsumeTransactionRange(TransactionValidationResults(), std::move(range), [](const auto& context) {
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
		TransactionValidationResults transactionValidationResults;
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
