#include "SequentialValidationPolicy.h"
#include "AggregateValidationResult.h"

namespace catapult { namespace validators {

	SequentialValidationPolicyFunc CreateSequentialValidationPolicy() {
		return [](const auto& entityInfos, const auto& validationFunctions) {
			ValidationResult aggregate = ValidationResult::Success;
			for (const auto& entityInfo : entityInfos) {
				for (const auto& validationFunction : validationFunctions) {
					auto result = validationFunction(entityInfo);
					AggregateValidationResult(aggregate, result);
					if (IsValidationResultFailure(aggregate))
						return aggregate;
				}
			}

			return aggregate;
		};
	}
}}
