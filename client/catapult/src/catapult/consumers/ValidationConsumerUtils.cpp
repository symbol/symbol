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

#include "ValidationConsumerUtils.h"
#include "ConsumerResultFactory.h"
#include "catapult/validators/AggregateValidationResult.h"

namespace catapult { namespace consumers {

	namespace {
		template<typename TExtractAndProcess>
		auto MakeValidationConsumer(TExtractAndProcess extractAndProcess) {
			return [extractAndProcess](auto& elements) {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				auto result = extractAndProcess(elements);
				if (IsValidationResultSuccess(result))
					return Continue();

				CATAPULT_LOG_LEVEL(validators::MapToLogLevel(result)) << "validation consumer failed: " << result;
				return Abort(result, disruptor::ConsumerResultSeverity::Fatal);
			};
		}
	}

	disruptor::ConstBlockConsumer MakeBlockValidationConsumer(
			const RequiresValidationPredicate& requiresValidationPredicate,
			const std::function<validators::ValidationResult (const model::WeakEntityInfos&)>& process) {
		return MakeValidationConsumer([requiresValidationPredicate, process](const auto& elements) {
			model::WeakEntityInfos entityInfos;
			ExtractMatchingEntityInfos(elements, entityInfos, requiresValidationPredicate);

			return process(entityInfos);
		});
	}

	disruptor::TransactionConsumer MakeTransactionValidationConsumer(
			const chain::FailedTransactionSink& failedTransactionSink,
			const std::function<std::vector<validators::ValidationResult> (model::WeakEntityInfos&)>& process) {
		return MakeValidationConsumer([failedTransactionSink, process](auto& elements) {
			model::WeakEntityInfos entityInfos;
			std::vector<size_t> entityInfoElementIndexes;
			ExtractEntityInfos(elements, entityInfos, entityInfoElementIndexes);

			auto results = process(entityInfos);

			auto numSkippedElements = 0u;
			auto aggregateResult = validators::ValidationResult::Success;
			for (auto i = 0u; i < results.size(); ++i) {
				auto result = results[i];
				validators::AggregateValidationResult(aggregateResult, result);
				if (IsValidationResultSuccess(result))
					continue;

				// notice that ExtractEntityInfos ignores skipped elements, so finding the index in elements for a
				// corresponding entityInfo requires an additional hop through entityInfoElementIndexes
				auto& element = elements[entityInfoElementIndexes[i]];
				element.ResultSeverity = disruptor::ConsumerResultSeverity::Neutral;
				++numSkippedElements;

				// only forward failure (not neutral) results
				if (IsValidationResultFailure(result)) {
					element.ResultSeverity = disruptor::ConsumerResultSeverity::Failure;
					failedTransactionSink(element.Transaction, element.EntityHash, result);
				}
			}

			// abort if an element failed
			if (0 == numSkippedElements)
				return validators::ValidationResult::Success;

			CATAPULT_LOG(trace) << "all " << numSkippedElements << " transaction(s) skipped in Transaction validation consumer";
			return aggregateResult;
		});
	}
}}
