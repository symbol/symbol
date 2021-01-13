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

#include "TransactionConsumers.h"
#include "ConsumerResultFactory.h"

namespace catapult { namespace consumers {

	namespace {
		class NewTransactionsConsumer {
		public:
			NewTransactionsConsumer(
					uint32_t minTransactionFailuresCountForBan,
					uint32_t minTransactionFailuresPercentForBan,
					const NewTransactionsProcessor& newTransactionsProcessor)
					: m_minTransactionFailuresCountForBan(minTransactionFailuresCountForBan)
					, m_minTransactionFailuresPercentForBan(minTransactionFailuresPercentForBan)
					, m_newTransactionsProcessor(newTransactionsProcessor)
			{}

		public:
			ConsumerResult operator()(disruptor::ConsumerInput& input) const {
				if (input.empty())
					return Abort(Failure_Consumer_Empty_Input);

				// 1. split up the input into its component transactions
				//    - detachTransactionRange transfers ownership of the range from the input
				//      but doesn't invalidate the input elements
				//    - the range is moved into ExtractEntitiesFromRange, which extends the lifetime of the range
				//      to the lifetime of the returned transactions
				auto transactions = model::TransactionRange::ExtractEntitiesFromRange(input.detachTransactionRange());

				// 2. prepare the output
				TransactionInfos transactionInfos;
				transactionInfos.reserve(transactions.size());

				// 3. filter transactions
				//    - the input elements are still valid even though the backing range has been detached
				size_t i = 0;
				size_t numFailures = 0;
				for (const auto& element : input.transactions()) {
					if (disruptor::ConsumerResultSeverity::Success == element.ResultSeverity) {
						transactionInfos.emplace_back(model::MakeTransactionInfo(transactions[i], element));
					} else if (disruptor::ConsumerResultSeverity::Failure == element.ResultSeverity) {
						++numFailures;
					}

					++i;
				}

				// 4. call the sink
				auto aggregateResult = m_newTransactionsProcessor(std::move(transactionInfos));
				aggregateResult.FailureCount += numFailures;

				// 5. indicate input was consumed and processing is complete
				if (0 == aggregateResult.FailureCount)
					return aggregateResult.SuccessCount > 0 ? CompleteSuccess() : CompleteNeutral();

				auto shouldBan =
						aggregateResult.FailureCount >= m_minTransactionFailuresCountForBan
						&& aggregateResult.FailureCount * 100 / transactions.size() >= m_minTransactionFailuresPercentForBan;
				return shouldBan
						? Abort(validators::ValidationResult::Failure, disruptor::ConsumerResultSeverity::Fatal)
						: Abort(validators::ValidationResult::Failure);
			}

		private:
			uint32_t m_minTransactionFailuresCountForBan;
			uint32_t m_minTransactionFailuresPercentForBan;
			NewTransactionsProcessor m_newTransactionsProcessor;
		};
	}

	disruptor::DisruptorConsumer CreateNewTransactionsConsumer(
			uint32_t minTransactionFailuresCountForBan,
			uint32_t minTransactionFailuresPercentForBan,
			const NewTransactionsProcessor& newTransactionsProcessor) {
		return NewTransactionsConsumer(minTransactionFailuresCountForBan, minTransactionFailuresPercentForBan, newTransactionsProcessor);
	}
}}
