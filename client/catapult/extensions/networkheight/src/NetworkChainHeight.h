#pragma once
#include "catapult/types.h"
#include <atomic>

namespace catapult { namespace networkheight {

	/// An atomic network chain height.
	using NetworkChainHeight = std::atomic<Height::ValueType>;
}}
