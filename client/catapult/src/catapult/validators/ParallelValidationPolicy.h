#pragma once
#include "ValidatorTypes.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace thread { class IoServiceThreadPool; } }

namespace catapult { namespace validators {

	/// A parallel validation policy that performs parallel validations on multiple threads.
	class ParallelValidationPolicy {
	public:
		virtual ~ParallelValidationPolicy() {}

	public:
		/// Validates all \a entityInfos using \a validationFunctions and short circuits on first failure.
		virtual thread::future<ValidationResult> validateShortCircuit(
				const model::WeakEntityInfos& entityInfos,
				const ValidationFunctions& validationFunctions) const = 0;

		/// Validates all \a entityInfos using \a validationFunctions and does \em NOT short circuit on failures.
		virtual thread::future<std::vector<ValidationResult>> validateAll(
				const model::WeakEntityInfos& entityInfos,
				const ValidationFunctions& validationFunctions) const = 0;
	};

	/// Creates a parallel validation policy using \a pPool for parallelization.
	std::shared_ptr<const ParallelValidationPolicy> CreateParallelValidationPolicy(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool);
}}
