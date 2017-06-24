#pragma once
#include "ParallelValidationPolicy.h"
#include "SequentialValidationPolicy.h"

namespace catapult { namespace validators {

	/// Creates a conditional validation policy around \a sequentialPolicyFunc and \a parallelPolicyFunc with a
	/// threshold of \a parallelThreshold.
	/// \note If the number of entities being validated exceeds \a parallelThreshold, the validation will be
	///       parallelized.
	SequentialValidationPolicyFunc CreateConditionalValidationPolicy(
			const SequentialValidationPolicyFunc& sequentialPolicyFunc,
			const ParallelValidationPolicyFunc& parallelPolicyFunc,
			uint32_t parallelThreshold);
}}
