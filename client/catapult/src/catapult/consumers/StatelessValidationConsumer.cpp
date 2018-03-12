#include "BlockConsumers.h"
#include "ConsumerResultFactory.h"
#include "InputUtils.h"
#include "TransactionConsumers.h"
#include "catapult/validators/AggregateEntityValidator.h"
#include "catapult/validators/AggregateValidationResult.h"

namespace catapult { namespace consumers {

	namespace {
		template<typename TExtractAndProcess>
		auto MakeConsumer(
				const std::shared_ptr<const validators::stateless::AggregateEntityValidator>& pValidator,
				TExtractAndProcess extractAndProcess) {
			validators::stateless::AggregateEntityValidator::DispatchForwarder dispatcher(pValidator->curry());
			return [pValidator, extractAndProcess, dispatcher](auto& elements) {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				auto result = extractAndProcess(elements, [&dispatcher](const auto& validationPolicyFunc, const auto& entityInfos) {
					return dispatcher.dispatch(validationPolicyFunc, entityInfos).get();
				});

				if (IsValidationResultSuccess(result))
					return Continue();

				CATAPULT_LOG_LEVEL(validators::MapToLogLevel(result)) << "stateless transaction validation failed: " << result;
				return Abort(result);
			};
		}

		auto AsShortCircuitFunction(const validators::ParallelValidationPolicy& policy) {
			return [&policy](const auto& entityInfos, const auto& validationFunctions) {
				return policy.validateShortCircuit(entityInfos, validationFunctions);
			};
		}

		auto AsAllFunction(const validators::ParallelValidationPolicy& policy) {
			return [&policy](const auto& entityInfos, const auto& validationFunctions) {
				return policy.validateAll(entityInfos, validationFunctions);
			};
		}
	}

	disruptor::ConstBlockConsumer CreateBlockStatelessValidationConsumer(
			const std::shared_ptr<const validators::stateless::AggregateEntityValidator>& pValidator,
			const std::shared_ptr<const validators::ParallelValidationPolicy>& pValidationPolicy,
			const RequiresValidationPredicate& requiresValidationPredicate) {
		return MakeConsumer(pValidator, [pValidationPolicy, requiresValidationPredicate](const auto& elements, auto dispatch) {
			model::WeakEntityInfos entityInfos;
			ExtractMatchingEntityInfos(elements, entityInfos, requiresValidationPredicate);

			return dispatch(AsShortCircuitFunction(*pValidationPolicy), entityInfos);
		});
	}

	disruptor::TransactionConsumer CreateTransactionStatelessValidationConsumer(
			const std::shared_ptr<const validators::stateless::AggregateEntityValidator>& pValidator,
			const std::shared_ptr<const validators::ParallelValidationPolicy>& pValidationPolicy,
			const chain::FailedTransactionSink& failedTransactionSink) {
		return MakeConsumer(pValidator, [pValidationPolicy, failedTransactionSink](auto& elements, auto dispatch) {
			model::WeakEntityInfos entityInfos;
			std::vector<size_t> entityInfoElementIndexes;
			ExtractEntityInfos(elements, entityInfos, entityInfoElementIndexes);

			auto results = dispatch(AsAllFunction(*pValidationPolicy), entityInfos);
			auto numSkippedElements = 0u;
			auto aggregateResult = validators::ValidationResult::Success;
			for (auto i = 0u; i < results.size(); ++i) {
				auto result = results[i];
				validators::AggregateValidationResult(aggregateResult, result);
				if (IsValidationResultSuccess(result))
					continue;

				// notice that ExtractEntityInfos ignores skipped elements, so finding the index in elements for a corresponding entityInfo
				// requires an additional hop through entityInfoElementIndexes
				auto& element = elements[entityInfoElementIndexes[i]];
				element.Skip = true;
				++numSkippedElements;

				// only forward failure (not neutral) results
				if (IsValidationResultFailure(result))
					failedTransactionSink(element.Transaction, element.EntityHash, result);
			}

			// only abort if all elements failed
			if (results.size() != numSkippedElements)
				return validators::ValidationResult::Success;

			CATAPULT_LOG(trace) << "all " << numSkippedElements << " transaction(s) skipped in TransactionStatelessValidation";
			return aggregateResult;
		});
	}
}}
