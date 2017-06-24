#include "BasicDispatcherService.h"
#include "LocalNodeStateRef.h"
#include "LocalNodeStats.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/thread/MultiServicePool.h"
#include "catapult/thread/Scheduler.h"
#include "catapult/utils/NetworkTime.h"

namespace catapult { namespace local {

	BasicDispatcherService::BasicDispatcherService(const utils::TimeSpan& transactionBatchPeriod)
			: m_transactionBatchPeriod(transactionBatchPeriod)
	{}

	BlockRangeConsumerFactoryFunc BasicDispatcherService::createBlockRangeConsumerFactory() {
		return [this](auto source) {
			return [this, source](auto&& range) {
				return m_pBlockDispatcher.lock()->processElement(disruptor::ConsumerInput(std::move(range), source));
			};
		};
	}

	CompletionAwareBlockRangeConsumerFactoryFunc BasicDispatcherService::createCompletionAwareBlockRangeConsumerFactory() {
		return [this](auto source) {
			return [this, source](auto&& range, const disruptor::ProcessingCompleteFunc& processingComplete) {
				return m_pBlockDispatcher.lock()->processElement(disruptor::ConsumerInput(std::move(range), source), processingComplete);
			};
		};
	}

	TransactionRangeConsumerFactoryFunc BasicDispatcherService::createTransactionRangeConsumerFactory() {
		return [this](auto source) {
			return [this, source](auto&& range) {
				m_pTransactionBatchRangeDispatcher->queue(std::move(range), source);
			};
		};
	}

	size_t BasicDispatcherService::numAddedBlockElements() const {
		return GetStatsValue(m_pBlockDispatcher.lock(), &disruptor::ConsumerDispatcher::numAddedElements);
	}

	size_t BasicDispatcherService::numAddedTransactionElements() const {
		return GetStatsValue(m_pTransactionDispatcher.lock(), &disruptor::ConsumerDispatcher::numAddedElements);
	}

	namespace {
		DispatcherStatus GetDispatcherStatus(const std::shared_ptr<disruptor::ConsumerDispatcher>& pDispatcher) {
			return !pDispatcher
					? DispatcherStatus{}
					: DispatcherStatus{ pDispatcher->name(), pDispatcher->size(), pDispatcher->isRunning() };
		}
	}

	DispatcherStatus BasicDispatcherService::blockDispatcherStatus() const {
		return GetDispatcherStatus(m_pBlockDispatcher.lock());
	}

	DispatcherStatus BasicDispatcherService::transactionDispatcherStatus() const {
		return GetDispatcherStatus(m_pTransactionDispatcher.lock());
	}

	void BasicDispatcherService::boot(
			thread::MultiServicePool& pool,
			std::unique_ptr<disruptor::ConsumerDispatcher>&& pBlockDispatcher,
			std::unique_ptr<disruptor::ConsumerDispatcher>&& pTransactionDispatcher) {
		// register the dispatchers with the pool so that they get shutdown in the proper order
		// (even though they do not use the pool threads)
		using ConsumerDispatcherPointer = std::shared_ptr<disruptor::ConsumerDispatcher>;
		auto pServiceGroup = pool.pushServiceGroup("dispatcher service");
		m_pBlockDispatcher = pServiceGroup->registerService(ConsumerDispatcherPointer(std::move(pBlockDispatcher)));
		m_pTransactionDispatcher = pServiceGroup->registerService(ConsumerDispatcherPointer(std::move(pTransactionDispatcher)));

		m_pTransactionBatchRangeDispatcher = std::make_unique<TransactionBatchRangeDispatcher>(*m_pTransactionDispatcher.lock());

		auto pScheduler = pServiceGroup->pushService(thread::CreateScheduler);
		pScheduler->addTask(createBatchTransactionTask());
	}

	thread::Task BasicDispatcherService::createBatchTransactionTask() const {
		thread::Task task;
		task.StartDelay = m_transactionBatchPeriod;
		task.RepeatDelay = m_transactionBatchPeriod;
		task.Name = "transaction batch task";

		task.Callback = [this]() {
			m_pTransactionBatchRangeDispatcher->dispatch();
			return thread::make_ready_future(thread::TaskResult::Continue);
		};

		return task;
	}

	std::unique_ptr<chain::UnconfirmedTransactionsUpdater> BasicDispatcherService::CreateUnconfirmedTransactionsUpdater(
			const LocalNodeStateRef& stateRef,
			cache::UtCache& unconfirmedTransactionsCache,
			const chain::ExecutionConfiguration& executionConfiguration) {
		return std::make_unique<chain::UnconfirmedTransactionsUpdater>(
				unconfirmedTransactionsCache,
				stateRef.Cache,
				executionConfiguration,
				&utils::NetworkTime);
	}
}}
