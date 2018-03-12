#include "DispatcherUtils.h"
#include "ServiceLocator.h"
#include "catapult/config/NodeConfiguration.h"
#include "catapult/disruptor/ConsumerDispatcher.h"
#include "catapult/subscribers/TransactionStatusSubscriber.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace extensions {

	consumers::HashCheckOptions CreateHashCheckOptions(const utils::TimeSpan& cacheDuration, const config::NodeConfiguration& nodeConfig) {
		return consumers::HashCheckOptions(
				cacheDuration.millis(),
				nodeConfig.ShortLivedCachePruneInterval.millis(),
				nodeConfig.ShortLivedCacheMaxSize);
	}

	chain::FailedTransactionSink SubscriberToSink(subscribers::TransactionStatusSubscriber& subscriber) {
		return [&subscriber](const auto& transaction, const auto& hash, auto result) {
			subscriber.notifyStatus(transaction, hash, utils::to_underlying_type(result));
		};
	}

	void AddDispatcherCounters(ServiceLocator& locator, const std::string& dispatcherName, const std::string& counterPrefix) {
		using disruptor::ConsumerDispatcher;

		locator.registerServiceCounter<ConsumerDispatcher>(dispatcherName, counterPrefix + " ELEM TOT", [](const auto& dispatcher) {
			return dispatcher.numAddedElements();
		});
		locator.registerServiceCounter<ConsumerDispatcher>(dispatcherName, counterPrefix + " ELEM ACT", [](const auto& dispatcher) {
			return dispatcher.numActiveElements();
		});
	}

	thread::Task CreateBatchTransactionTask(TransactionBatchRangeDispatcher& dispatcher, const std::string& name) {
		return thread::CreateNamedTask("batch " + name + " task", [&dispatcher]() {
			dispatcher.dispatch();
			return thread::make_ready_future(thread::TaskResult::Continue);
		});
	}
}}
