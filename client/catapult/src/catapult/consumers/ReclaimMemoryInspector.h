#pragma once
#include "catapult/disruptor/DisruptorInspector.h"

namespace catapult { namespace consumers {

	/// Creates an inspector that reclaims memory.
	disruptor::DisruptorInspector CreateReclaimMemoryInspector();
}}
