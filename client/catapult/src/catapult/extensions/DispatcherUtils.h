/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

	/// Transaction batch range dispatcher.
	using TransactionBatchRangeDispatcher = disruptor::BatchRangeDispatcher<model::AnnotatedTransactionRange>;

	/// Creates a task with \a name that dispatches all transactions batched in \a dispatcher.
	thread::Task CreateBatchTransactionTask(TransactionBatchRangeDispatcher& dispatcher, const std::string& name);
}}
