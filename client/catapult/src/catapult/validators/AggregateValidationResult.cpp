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

#include "AggregateValidationResult.h"

namespace catapult { namespace validators {

	namespace {
		struct AtomicAssignPolicy {
			using AggregateType = std::atomic<ValidationResult>;

			static ValidationResult Unwrap(AggregateType& aggregate) {
				return aggregate.load();
			}

			static bool CompareExchange(AggregateType& aggregate, ValidationResult current, ValidationResult value) {
				return aggregate.compare_exchange_strong(current, value);
			}
		};

		struct EnumAssignPolicy {
			using AggregateType = ValidationResult;

			static ValidationResult Unwrap(AggregateType& aggregate) {
				return aggregate;
			}

			static bool CompareExchange(AggregateType& aggregate, ValidationResult, ValidationResult value) {
				// policy is only supported for single threaded environment, so always exchange
				aggregate = value;
				return true;
			}
		};

		template<typename TAssignPolicy>
		void AggregateResult(typename TAssignPolicy::AggregateType& aggregate, ValidationResult value) {
			for (;;) {
				// only change aggregate value if new value increases severity
				auto current = TAssignPolicy::Unwrap(aggregate);
				if (GetSeverity(current) >= GetSeverity(value))
					return;

				if (TAssignPolicy::CompareExchange(aggregate, current, value))
					return;
			}
		}
	}

	void AggregateValidationResult(std::atomic<ValidationResult>& aggregate, ValidationResult value) {
		AggregateResult<AtomicAssignPolicy>(aggregate, value);
	}

	void AggregateValidationResult(ValidationResult& aggregate, ValidationResult value) {
		AggregateResult<EnumAssignPolicy>(aggregate, value);
	}
}}
