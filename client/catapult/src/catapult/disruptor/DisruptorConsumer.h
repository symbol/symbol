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

#pragma once
#include "DisruptorElement.h"
#include <functional>

namespace catapult { namespace disruptor {

	/// A typed disruptor consumer function.
	template<typename TInput>
	using DisruptorConsumerT = std::function<ConsumerResult (TInput&)>;

	/// A disruptor consumer function.
	using DisruptorConsumer = DisruptorConsumerT<ConsumerInput>;

	/// A const disruptor consumer function.
	using ConstDisruptorConsumer = DisruptorConsumerT<const ConsumerInput>;

	/// A block disruptor consumer function.
	using BlockConsumer = DisruptorConsumerT<BlockElements>;

	/// A const block disruptor consumer function.
	using ConstBlockConsumer = DisruptorConsumerT<const BlockElements>;

	/// A transaction disruptor consumer function.
	using TransactionConsumer = DisruptorConsumerT<TransactionElements>;

	/// A const transaction disruptor consumer function.
	using ConstTransactionConsumer = DisruptorConsumerT<const TransactionElements>;

	/// Maps \a blockConsumers to disruptor consumers so that they can be used to create a ConsumerDispatcher.
	std::vector<DisruptorConsumer> DisruptorConsumersFromBlockConsumers(const std::vector<BlockConsumer>& blockConsumers);

	/// Maps \a transactionConsumers to disruptor consumers so that they can be used to create a ConsumerDispatcher.
	std::vector<DisruptorConsumer> DisruptorConsumersFromTransactionConsumers(
			const std::vector<TransactionConsumer>& transactionConsumers);
}}
