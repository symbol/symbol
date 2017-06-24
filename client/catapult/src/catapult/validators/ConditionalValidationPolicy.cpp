#include "ConditionalValidationPolicy.h"

namespace catapult { namespace validators {

	SequentialValidationPolicyFunc CreateConditionalValidationPolicy(
			const SequentialValidationPolicyFunc& sequentialPolicyFunc,
			const ParallelValidationPolicyFunc& parallelPolicyFunc,
			uint32_t parallelThreshold) {
		return [sequentialPolicyFunc, parallelPolicyFunc, parallelThreshold](
				const auto& entityInfos,
				const auto& validationFunctions) {
			return entityInfos.size() > parallelThreshold
					? parallelPolicyFunc(entityInfos, validationFunctions).get()
					: sequentialPolicyFunc(entityInfos, validationFunctions);
		};
	}
}}
