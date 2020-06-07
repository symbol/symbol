/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#pragma once
#include "ValidatorTypes.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace thread { class IoThreadPool; } }

namespace catapult { namespace validators {

	/// Parallel validation policy that performs parallel (stateless) validations on multiple threads.
	class ParallelValidationPolicy {
	public:
		virtual ~ParallelValidationPolicy() = default;

	public:
		/// Validates all \a entityInfos and short circuits on first failure.
		virtual thread::future<ValidationResult> validateShortCircuit(const model::WeakEntityInfos& entityInfos) const = 0;

		/// Validates all \a entityInfos and does \em NOT short circuit on failures.
		virtual thread::future<std::vector<ValidationResult>> validateAll(const model::WeakEntityInfos& entityInfos) const = 0;
	};

	/// Creates a parallel validation policy using \a pool for parallelization and \a pValidator for validation.
	std::shared_ptr<const ParallelValidationPolicy> CreateParallelValidationPolicy(
			thread::IoThreadPool& pool,
			const std::shared_ptr<const StatelessEntityValidator>& pValidator);
}}
