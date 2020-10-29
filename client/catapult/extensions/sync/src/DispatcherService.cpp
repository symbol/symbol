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

#include "DispatcherService.h"
#include "DispatcherSyncHandlers.h"
#include "PredicateUtils.h"
#include "RollbackInfo.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/BlockStatisticCache.h"
#include "catapult/cache_core/ImportanceView.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/chain/ChainUtils.h"
#include "catapult/chain/UtUpdater.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/consumers/AuditConsumer.h"
#include "catapult/consumers/BlockConsumers.h"
#include "catapult/consumers/ConsumerUtils.h"
#include "catapult/consumers/ReclaimMemoryInspector.h"
#include "catapult/consumers/TransactionConsumers.h"
#include "catapult/consumers/UndoBlock.h"
#include "catapult/crypto/SecureRandomGenerator.h"
#include "catapult/disruptor/BatchRangeDispatcher.h"
#include "catapult/extensions/CommitStepHandler.h"
#include "catapult/extensions/DispatcherUtils.h"
#include "catapult/extensions/ExecutionConfigurationFactory.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/NodeInteractionUtils.h"
#include "catapult/extensions/PluginUtils.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/ionet/NodeInteractionResult.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/subscribers/NodeSubscriber.h"
#include "catapult/subscribers/StateChangeSubscriber.h"
#include "catapult/subscribers/TransactionStatusSubscriber.h"
#include "catapult/thread/MultiServicePool.h"
#include <boost/filesystem.hpp>

using namespace catapult::consumers;
using namespace catapult::disruptor;

namespace catapult { namespace sync {

	namespace {
		// region utils

		crypto::RandomFiller CreateRandomFiller() {
			return [](auto* pOut, auto count) {
				crypto::SecureRandomGenerator().fill(pOut, count);
			};
		}

		std::shared_ptr<const validators::ParallelValidationPolicy> CreateParallelValidationPolicy(
				thread::IoThreadPool& validatorPool,
				const plugins::PluginManager& pluginManager) {
			return validators::CreateParallelValidationPolicy(
					validatorPool,
					extensions::CreateStatelessEntityValidator(pluginManager, model::SignatureNotification::Notification_Type));
		}

		ConsumerDispatcherOptions CreateBlockConsumerDispatcherOptions(const config::NodeConfiguration& config) {
			auto options = ConsumerDispatcherOptions("block dispatcher", config.BlockDisruptorSize);
			options.ElementTraceInterval = config.BlockElementTraceInterval;
			options.ShouldThrowWhenFull = config.EnableDispatcherAbortWhenFull;
			return options;
		}

		ConsumerDispatcherOptions CreateTransactionConsumerDispatcherOptions(const config::NodeConfiguration& config) {
			auto options = ConsumerDispatcherOptions("transaction dispatcher", config.TransactionDisruptorSize);
			options.ElementTraceInterval = config.TransactionElementTraceInterval;
			options.ShouldThrowWhenFull = config.EnableDispatcherAbortWhenFull;
			return options;
		}

		std::unique_ptr<ConsumerDispatcher> CreateConsumerDispatcher(
				extensions::ServiceState& state,
				const ConsumerDispatcherOptions& options,
				std::vector<DisruptorConsumer>&& disruptorConsumers) {
			auto& nodeSubscriber = state.nodeSubscriber();
			auto& statusSubscriber = state.transactionStatusSubscriber();
			auto reclaimMemoryInspector = CreateReclaimMemoryInspector();
			const auto& localNetworks = state.config().Node.LocalNetworks;
			auto inspector = [&nodeSubscriber, &statusSubscriber, &nodes = state.nodes(), &localNetworks, reclaimMemoryInspector](
					auto& input,
					const auto& completionResult) {
				statusSubscriber.flush();
				auto interactionResult = consumers::ToNodeInteractionResult(input.sourceIdentity(), completionResult);
				extensions::IncrementNodeInteraction(nodes, interactionResult);

				nodes.modifier().pruneBannedNodes();

				if (ConsumerResultSeverity::Fatal == completionResult.ResultSeverity) {
					const auto& identity = input.sourceIdentity();
					if (config::IsLocalHost(identity.Host, localNetworks))
						CATAPULT_LOG(debug) << "bypassing banning of " << identity << " because host is contained in local networks";
					else
						nodeSubscriber.notifyBan(identity, completionResult.CompletionCode);
				}

				reclaimMemoryInspector(input, completionResult);
			};

			// if enabled, add an audit consumer before all other consumers
			const auto& config = state.config();
			if (config.Node.EnableDispatcherInputAuditing) {
				auto auditPath = boost::filesystem::path(config.User.DataDirectory) / "audit" / std::string(options.DispatcherName);
				auditPath /= std::to_string(state.timeSupplier()().unwrap());
				CATAPULT_LOG(debug) << "enabling auditing to " << auditPath;

				config::CatapultDirectory(auditPath).createAll();
				disruptorConsumers.insert(disruptorConsumers.begin(), CreateAuditConsumer(auditPath.generic_string()));
			}

			return std::make_unique<ConsumerDispatcher>(options, disruptorConsumers, inspector);
		}

		// endregion

		// region block

		ReceiptValidationMode GetReceiptValidationMode(const model::BlockChainConfiguration& blockChainConfig) {
			return blockChainConfig.EnableVerifiableReceipts ? ReceiptValidationMode::Enabled : ReceiptValidationMode::Disabled;
		}

		BlockChainProcessor CreateSyncProcessor(
				const model::BlockChainConfiguration& blockChainConfig,
				const chain::ExecutionConfiguration& executionConfig) {
			BlockHitPredicateFactory blockHitPredicateFactory = [&blockChainConfig](const cache::ReadOnlyCatapultCache& cache) {
				cache::ImportanceView view(cache.sub<cache::AccountStateCache>());
				return chain::BlockHitPredicate(blockChainConfig, [view](const auto& publicKey, auto height) {
					return view.getAccountImportanceOrDefault(publicKey, height);
				});
			};
			return CreateBlockChainProcessor(
					blockHitPredicateFactory,
					chain::CreateBatchEntityProcessor(executionConfig),
					GetReceiptValidationMode(blockChainConfig));
		}

		BlockChainSyncHandlers CreateBlockChainSyncHandlers(extensions::ServiceState& state, RollbackInfo& rollbackInfo) {
			const auto& blockChainConfig = state.config().BlockChain;
			const auto& pluginManager = state.pluginManager();

			BlockChainSyncHandlers syncHandlers;
			syncHandlers.DifficultyChecker = [&rollbackInfo, blockChainConfig](const auto& blocks, const cache::CatapultCache& cache) {
				auto result = chain::CheckDifficulties(cache.sub<cache::BlockStatisticCache>(), blocks, blockChainConfig);
				rollbackInfo.modifier().reset();
				return blocks.size() == result;
			};

			syncHandlers.LocalFinalizedHeightHashPairSupplier = extensions::CreateLocalFinalizedHeightHashPairSupplier(state);

			auto pUndoObserver = utils::UniqueToShared(extensions::CreateUndoEntityObserver(pluginManager));
			syncHandlers.UndoBlock = [&rollbackInfo, &pluginManager, pUndoObserver](
					const auto& blockElement,
					auto& observerState,
					auto undoBlockType) {
				rollbackInfo.modifier().increment();
				auto readOnlyCache = observerState.Cache.toReadOnly();
				auto resolverContext = pluginManager.createResolverContext(readOnlyCache);
				UndoBlock(blockElement, { *pUndoObserver, resolverContext, observerState }, undoBlockType);
			};
			syncHandlers.Processor = CreateSyncProcessor(blockChainConfig, extensions::CreateExecutionConfiguration(pluginManager));

			syncHandlers.StateChange = [&rollbackInfo, &localScore = state.score(), &subscriber = state.stateChangeSubscriber()](
					const auto& changeInfo) {
				localScore += changeInfo.ScoreDelta;

				// note: changeInfo contains only score delta, subscriber will get both current local score and changeInfo
				subscriber.notifyScoreChange(localScore.get());
				subscriber.notifyStateChange(changeInfo);

				rollbackInfo.modifier().save();
			};

			auto dataDirectory = config::CatapultDataDirectory(state.config().User.DataDirectory);
			syncHandlers.PreStateWritten = [](const auto&, auto) {};
			syncHandlers.TransactionsChange = state.hooks().transactionsChangeHandler();
			syncHandlers.CommitStep = extensions::CreateCommitStepHandler(dataDirectory);

			if (state.config().Node.EnableCacheDatabaseStorage)
				AddSupplementalDataResiliency(syncHandlers, dataDirectory, state.cache(), state.score());

			return syncHandlers;
		}

		class BlockDispatcherBuilder {
		public:
			explicit BlockDispatcherBuilder(extensions::ServiceState& state)
					: m_state(state)
					, m_nodeConfig(m_state.config().Node)
			{}

		public:
			void addHashConsumers() {
				m_consumers.push_back(CreateBlockHashCalculatorConsumer(
						m_state.config().BlockChain.Network.GenerationHashSeed,
						m_state.pluginManager().transactionRegistry()));
				m_consumers.push_back(CreateBlockHashCheckConsumer(
						m_state.timeSupplier(),
						extensions::CreateHashCheckOptions(m_nodeConfig.ShortLivedCacheBlockDuration, m_nodeConfig)));
			}

			std::shared_ptr<ConsumerDispatcher> build(thread::IoThreadPool& validatorPool, RollbackInfo& rollbackInfo) {
				const auto& utCache = const_cast<const extensions::ServiceState&>(m_state).utCache();
				auto requiresValidationPredicate = ToRequiresValidationPredicate(m_state.hooks().knownHashPredicate(utCache));
				m_consumers.push_back(CreateBlockChainCheckConsumer(
						m_nodeConfig.MaxBlocksPerSyncAttempt,
						m_state.config().BlockChain.MaxBlockFutureTime,
						m_state.timeSupplier()));
				m_consumers.push_back(CreateBlockStatelessValidationConsumer(
						CreateParallelValidationPolicy(validatorPool, m_state.pluginManager()),
						requiresValidationPredicate));
				m_consumers.push_back(CreateBlockBatchSignatureConsumer(
						m_state.config().BlockChain.Network.GenerationHashSeed,
						CreateRandomFiller(),
						m_state.pluginManager().createNotificationPublisher(),
						validatorPool,
						requiresValidationPredicate));

				auto disruptorConsumers = DisruptorConsumersFromBlockConsumers(m_consumers);
				disruptorConsumers.push_back(CreateBlockChainSyncConsumer(
						m_state.cache(),
						m_state.storage(),
						CreateBlockChainSyncHandlers(m_state, rollbackInfo)));

				if (m_state.config().Node.EnableAutoSyncCleanup)
					disruptorConsumers.push_back(CreateBlockChainSyncCleanupConsumer(m_state.config().User.DataDirectory));

				// forward locally harvested blocks and blocks pushed by partners
				auto newBlockSinkSourceMask = static_cast<InputSource>(
						utils::to_underlying_type(InputSource::Local)
						| utils::to_underlying_type(InputSource::Remote_Push));
				disruptorConsumers.push_back(CreateNewBlockConsumer(m_state.hooks().newBlockSink(), newBlockSinkSourceMask));
				return CreateConsumerDispatcher(
						m_state,
						CreateBlockConsumerDispatcherOptions(m_nodeConfig),
						std::move(disruptorConsumers));
			}

		private:
			extensions::ServiceState& m_state;
			const config::NodeConfiguration& m_nodeConfig;
			std::vector<BlockConsumer> m_consumers;
		};

		void RegisterBlockDispatcherService(
				const std::shared_ptr<ConsumerDispatcher>& pDispatcher,
				thread::MultiServicePool::ServiceGroup& serviceGroup,
				extensions::ServiceLocator& locator,
				extensions::ServiceState& state) {
			serviceGroup.registerService(pDispatcher);
			locator.registerService("dispatcher.block", pDispatcher);

			state.hooks().setBlockRangeConsumerFactory([&dispatcher = *pDispatcher, &nodes = state.nodes()](auto source) {
				return [&dispatcher, &nodes, source](auto&& range) {
					if (!nodes.view().isBanned(range.SourceIdentity))
						dispatcher.processElement(ConsumerInput(std::move(range), source));
				};
			});

			state.hooks().setCompletionAwareBlockRangeConsumerFactory([&dispatcher = *pDispatcher, &nodes = state.nodes()](auto source) {
				return [&dispatcher, &nodes, source](auto&& range, const auto& processingComplete) {
					return disruptor::InputSource::Local == source || !nodes.view().isBanned(range.SourceIdentity)
							? dispatcher.processElement(ConsumerInput(std::move(range), source), processingComplete)
							: 0;
				};
			});
		}

		// endregion

		// region transaction

		class TransactionDispatcherBuilder {
		public:
			explicit TransactionDispatcherBuilder(extensions::ServiceState& state)
					: m_state(state)
					, m_nodeConfig(m_state.config().Node)
			{}

		public:
			void addHashConsumers() {
				const auto& utCache = const_cast<const extensions::ServiceState&>(m_state).utCache();
				m_consumers.push_back(CreateTransactionHashCalculatorConsumer(
						m_state.config().BlockChain.Network.GenerationHashSeed,
						m_state.pluginManager().transactionRegistry()));
				m_consumers.push_back(CreateTransactionHashCheckConsumer(
						m_state.timeSupplier(),
						extensions::CreateHashCheckOptions(m_nodeConfig.ShortLivedCacheTransactionDuration, m_nodeConfig),
						m_state.hooks().knownHashPredicate(utCache)));
			}

			std::shared_ptr<ConsumerDispatcher> build(thread::IoThreadPool& validatorPool, chain::UtUpdater& utUpdater) {
				auto failedTransactionSink = extensions::SubscriberToSink(m_state.transactionStatusSubscriber());
				m_consumers.push_back(CreateTransactionStatelessValidationConsumer(
						CreateParallelValidationPolicy(validatorPool, m_state.pluginManager()),
						failedTransactionSink));
				m_consumers.push_back(CreateTransactionBatchSignatureConsumer(
						m_state.config().BlockChain.Network.GenerationHashSeed,
						CreateRandomFiller(),
						m_state.pluginManager().createNotificationPublisher(),
						validatorPool,
						failedTransactionSink));

				auto disruptorConsumers = DisruptorConsumersFromTransactionConsumers(m_consumers);
				disruptorConsumers.push_back(CreateNewTransactionsConsumer(
						[&utUpdater, newTransactionsSink = m_state.hooks().newTransactionsSink()](auto&& transactionInfos) {
					// Note that all transaction infos are broadcast even though some transactions might fail stateful validation because:
					// 1. even though a transaction can fail stateful validation on one node, it might pass the validation on another
					// 2. if the node is not synced it might reject many transactions that are perfectly valid due to missing account
					//    state information
					newTransactionsSink(transactionInfos);
					utUpdater.update(std::move(transactionInfos));
				}));

				return CreateConsumerDispatcher(
						m_state,
						CreateTransactionConsumerDispatcherOptions(m_nodeConfig),
						std::move(disruptorConsumers));
			}

		private:
			extensions::ServiceState& m_state;
			const config::NodeConfiguration& m_nodeConfig;
			std::vector<TransactionConsumer> m_consumers;
		};

		void RegisterTransactionDispatcherService(
				const std::shared_ptr<ConsumerDispatcher>& pDispatcher,
				thread::MultiServicePool::ServiceGroup& serviceGroup,
				extensions::ServiceLocator& locator,
				extensions::ServiceState& state) {
			serviceGroup.registerService(pDispatcher);
			locator.registerService("dispatcher.transaction", pDispatcher);

			auto pBatchRangeDispatcher = std::make_shared<extensions::TransactionBatchRangeDispatcher>(
					*pDispatcher,
					state.config().BlockChain.Network.NodeEqualityStrategy);
			locator.registerRootedService("dispatcher.transaction.batch", pBatchRangeDispatcher);

			state.hooks().setTransactionRangeConsumerFactory([&dispatcher = *pBatchRangeDispatcher, &nodes = state.nodes()](auto source) {
				return [&dispatcher, &nodes, source](auto&& range) {
					if (!nodes.view().isBanned(range.SourceIdentity))
						dispatcher.queue(std::move(range), source);
				};
			});

			state.tasks().push_back(extensions::CreateBatchTransactionTask(*pBatchRangeDispatcher, "transaction"));
		}

		// endregion

		chain::UtUpdater& CreateAndRegisterUtUpdater(extensions::ServiceLocator& locator, extensions::ServiceState& state) {
			auto pUtUpdater = std::make_shared<chain::UtUpdater>(
					state.utCache(),
					state.cache(),
					state.config().Node.MinFeeMultiplier,
					extensions::CreateExecutionConfiguration(state.pluginManager()),
					state.timeSupplier(),
					extensions::SubscriberToSink(state.transactionStatusSubscriber()),
					CreateUtUpdaterThrottle(state.config()));
			locator.registerRootedService("dispatcher.utUpdater", pUtUpdater);

			auto& utUpdater = *pUtUpdater;
			state.hooks().addTransactionsChangeHandler([&utUpdater](const auto& changeInfo) {
				utUpdater.update(changeInfo.AddedTransactionHashes, changeInfo.RevertedTransactionInfos);
			});

			return utUpdater;
		}

		auto CreateAndRegisterRollbackService(
				extensions::ServiceLocator& locator,
				const chain::TimeSupplier& timeSupplier,
				const model::BlockChainConfiguration& config) {
			auto rollbackDurationFull = CalculateTransactionCacheDuration(config);
			auto rollbackDurationHalf = utils::TimeSpan::FromMilliseconds(rollbackDurationFull.millis() / 2);
			auto pRollbackInfo = std::make_shared<RollbackInfo>(timeSupplier, rollbackDurationHalf);
			locator.registerRootedService("rollbacks", pRollbackInfo);
			return pRollbackInfo;
		}

		void AddRollbackCounter(
				extensions::ServiceLocator& locator,
				const std::string& counterName,
				RollbackResult rollbackResult,
				RollbackCounterType rollbackCounterType) {
			locator.registerServiceCounter<RollbackInfo>("rollbacks", counterName, [rollbackResult, rollbackCounterType](
					const auto& rollbackInfo) {
				return rollbackInfo.view().counter(rollbackResult, rollbackCounterType);
			});
		}

		class DispatcherServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "Dispatcher", extensions::ServiceRegistrarPhase::Post_Remote_Peers };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				extensions::AddDispatcherCounters(locator, "dispatcher.block", "BLK");
				extensions::AddDispatcherCounters(locator, "dispatcher.transaction", "TX");

				AddRollbackCounter(locator, "RB COMMIT ALL", RollbackResult::Committed, RollbackCounterType::All);
				AddRollbackCounter(locator, "RB COMMIT RCT", RollbackResult::Committed, RollbackCounterType::Recent);
				AddRollbackCounter(locator, "RB IGNORE ALL", RollbackResult::Ignored, RollbackCounterType::All);
				AddRollbackCounter(locator, "RB IGNORE RCT", RollbackResult::Ignored, RollbackCounterType::Recent);
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				// create shared services
				auto* pValidatorPool = state.pool().pushIsolatedPool("validator");
				auto& utUpdater = CreateAndRegisterUtUpdater(locator, state);

				// create the block and transaction dispatchers and related services
				// (notice that the dispatcher service group must be after the validator isolated pool in order to allow proper shutdown)
				auto pServiceGroup = state.pool().pushServiceGroup("dispatcher service");

				BlockDispatcherBuilder blockDispatcherBuilder(state);
				blockDispatcherBuilder.addHashConsumers();

				TransactionDispatcherBuilder transactionDispatcherBuilder(state);
				transactionDispatcherBuilder.addHashConsumers();

				auto pRollbackInfo = CreateAndRegisterRollbackService(locator, state.timeSupplier(), state.config().BlockChain);
				auto pBlockDispatcher = blockDispatcherBuilder.build(*pValidatorPool, *pRollbackInfo);
				RegisterBlockDispatcherService(pBlockDispatcher, *pServiceGroup, locator, state);

				auto pTransactionDispatcher = transactionDispatcherBuilder.build(*pValidatorPool, utUpdater);
				RegisterTransactionDispatcherService(pTransactionDispatcher, *pServiceGroup, locator, state);
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(Dispatcher)() {
		return std::make_unique<DispatcherServiceRegistrar>();
	}
}}
