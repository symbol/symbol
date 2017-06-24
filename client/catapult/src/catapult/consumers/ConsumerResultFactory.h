#pragma once
#include "ConsumerResults.h"
#include "catapult/disruptor/DisruptorTypes.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace consumers {

	/// Creates a continuation consumer result.
	constexpr disruptor::ConsumerResult Continue() {
		return disruptor::ConsumerResult::Continue();
	}

	/// Creates an aborted consumer result around \a validationResult.
	constexpr disruptor::ConsumerResult Abort(validators::ValidationResult validationResult) {
		return disruptor::ConsumerResult::Abort(utils::to_underlying_type(validationResult));
	}

	/// Creates a completed consumer result.
	constexpr disruptor::ConsumerResult Complete() {
		return disruptor::ConsumerResult::Complete();
	}
}}
