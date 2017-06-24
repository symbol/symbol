#pragma once
#include "ValidatorTypes.h"
#include "catapult/thread/Future.h"
#include "catapult/utils/WrappedWithOwnerDecorator.h"
#include "catapult/exceptions.h"

namespace catapult { namespace thread { class IoServiceThreadPool; } }

namespace catapult { namespace validators {

	/// Wraps a parallel validation function and its owning object.
	/// \note The owning object needs to be \em exposed instead of simply captured in order to allow deterministic
	///       shutdown.
	class ParallelValidationPolicyFunc
			: public utils::ResettableWrappedWithOwnerDecorator<ValidationPolicyFunc<thread::future<ValidationResult>>> {
	public:
		/// Creates an empty function.
		ParallelValidationPolicyFunc()
				: ParallelValidationPolicyFunc(ValidationPolicyFunc<thread::future<ValidationResult>>(), nullptr)
		{}

		/// Creates a function around \a func with \a pOwner.
		ParallelValidationPolicyFunc(
				const ValidationPolicyFunc<thread::future<ValidationResult>>& func,
				const std::shared_ptr<const void>& pOwner)
				: utils::ResettableWrappedWithOwnerDecorator<ValidationPolicyFunc<thread::future<ValidationResult>>>(
						func,
						pOwner)
		{}
	};

	/// Creates a parallel validation policy.
	ParallelValidationPolicyFunc CreateParallelValidationPolicy(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool);
}}
