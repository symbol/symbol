#pragma once
#include "catapult/chain/ChainFunctions.h"
#include "catapult/consumers/HashCheckOptions.h"
#include "catapult/disruptor/BatchRangeDispatcher.h"
#include "catapult/thread/Task.h"

namespace catapult {
	namespace config { struct NodeConfiguration; }
	namespace extensions { class ServiceLocator; }
	namespace subscribers { class TransactionStatusSubscriber; }
}

namespace catapult { namespace extensions {

	/// Creates hash check options based on \a cacheDuration and \a nodeConfig.
	consumers::HashCheckOptions CreateHashCheckOptions(const utils::TimeSpan& cacheDuration, const config::NodeConfiguration& nodeConfig);

	/// Converts \a subscriber to a sink.
	chain::FailedTransactionSink SubscriberToSink(subscribers::TransactionStatusSubscriber& subscriber);

	/// Adds dispatcher counters with prefix \a counterPrefix to \a locator for a dispatcher named \a dispatcherName.
	void AddDispatcherCounters(ServiceLocator& locator, const std::string& dispatcherName, const std::string& counterPrefix);

	/// A transaction batch range dispatcher.
	using TransactionBatchRangeDispatcher = disruptor::BatchRangeDispatcher<model::AnnotatedTransactionRange>;

	/// Creates a task with \a name that dispatches all transactions batched in \a dispatcher.
	thread::Task CreateBatchTransactionTask(TransactionBatchRangeDispatcher& dispatcher, const std::string& name);
}}
