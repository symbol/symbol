#include "AggregateValidationResult.h"

namespace catapult { namespace validators {

	namespace {
		struct AtomicAssignPolicy {
			using AggregateType = std::atomic<ValidationResult>;

			static ValidationResult Unwrap(AggregateType& aggregate) {
				return aggregate.load();
			}

			static void CompareExchange(AggregateType& aggregate, ValidationResult current, ValidationResult value) {
				aggregate.compare_exchange_strong(current, value);
			}
		};

		struct EnumAssignPolicy {
			using AggregateType = ValidationResult;

			static ValidationResult Unwrap(AggregateType& aggregate) {
				return aggregate;
			}

			static void CompareExchange(AggregateType& aggregate, ValidationResult current, ValidationResult value) {
				if (aggregate == current)
					aggregate = value;
			}
		};

		template<typename TAssignPolicy>
		void AggregateResult(typename TAssignPolicy::AggregateType& aggregate, ValidationResult value) {
			// only change aggregate value if new value increases severity
			auto current = TAssignPolicy::Unwrap(aggregate);
			if (GetSeverity(current) >= GetSeverity(value))
				return;

			TAssignPolicy::CompareExchange(aggregate, current, value);
		}
	}

	void AggregateValidationResult(std::atomic<ValidationResult>& aggregate, ValidationResult value) {
		AggregateResult<AtomicAssignPolicy>(aggregate, value);
	}

	void AggregateValidationResult(ValidationResult& aggregate, ValidationResult value) {
		AggregateResult<EnumAssignPolicy>(aggregate, value);
	}
}}
