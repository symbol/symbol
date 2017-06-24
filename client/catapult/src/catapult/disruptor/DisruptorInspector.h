#pragma once
#include "DisruptorElement.h"
#include <functional>

namespace catapult { namespace disruptor {

	/// A disruptor inspector function.
	using DisruptorInspector = std::function<void (ConsumerInput&, const ConsumerCompletionResult&)>;
}}
