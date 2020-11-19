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

#pragma once
#include "ConsumerResults.h"
#include "catapult/disruptor/DisruptorTypes.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace consumers {

	/// Creates a continuation consumer result.
	constexpr disruptor::ConsumerResult Continue() {
		return disruptor::ConsumerResult::Continue();
	}

	/// Creates an aborted consumer result around \a validationResult with \a severity.
	constexpr disruptor::ConsumerResult Abort(validators::ValidationResult validationResult, disruptor::ConsumerResultSeverity severity) {
		return disruptor::ConsumerResult::Abort(utils::to_underlying_type(validationResult), severity);
	}

	/// Creates an aborted consumer result around \a validationResult.
	constexpr disruptor::ConsumerResult Abort(validators::ValidationResult validationResult) {
		auto validationResultSeverity = validators::GetSeverity(validationResult);
		switch (validationResultSeverity) {
		case validators::ResultSeverity::Failure:
			return Abort(validationResult, disruptor::ConsumerResultSeverity::Failure);

		case validators::ResultSeverity::Neutral:
			return Abort(validationResult, disruptor::ConsumerResultSeverity::Neutral);

		default:
			return Abort(validationResult, disruptor::ConsumerResultSeverity::Success);
		}
	}

	/// Creates a completed success consumer result.
	constexpr disruptor::ConsumerResult CompleteSuccess() {
		auto code = utils::to_underlying_type(validators::ValidationResult::Success);
		return disruptor::ConsumerResult::Complete(code, disruptor::ConsumerResultSeverity::Success);
	}

	/// Creates a completed neutral consumer result.
	constexpr disruptor::ConsumerResult CompleteNeutral() {
		auto code = utils::to_underlying_type(validators::ValidationResult::Neutral);
		return disruptor::ConsumerResult::Complete(code, disruptor::ConsumerResultSeverity::Neutral);
	}
}}
