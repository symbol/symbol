#pragma once
#include <stdint.h>

namespace catapult { namespace model {

	/// Supply change directions.
	enum class MosaicSupplyChangeDirection : uint8_t {
		/// Decreases the supply.
		Decrease,
		/// Increases the supply.
		Increase
	};
}}
