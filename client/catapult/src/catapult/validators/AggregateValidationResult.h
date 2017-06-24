#pragma once
#include "ValidationResult.h"
#include <atomic>

namespace catapult { namespace validators {

	/// Aggregates \a result into \a aggregate.
	void AggregateValidationResult(std::atomic<ValidationResult>& aggregate, ValidationResult value);

	/// Aggregates \a result into \a aggregate.
	void AggregateValidationResult(ValidationResult& aggregate, ValidationResult value);
}}
