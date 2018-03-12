#pragma once
#include <stdint.h>

namespace catapult { namespace model {

	/// Possible facility codes.
	enum class FacilityCode : uint8_t {
		/// Aggregate facility code.
		Aggregate = 0x41,
		/// Core facility code.
		Core = 0x43,
		/// Lock facility code.
		Lock = 0x4C,
		/// Mosaic facility code.
		Mosaic = 0x4D,
		/// Multisig facility code.
		Multisig = 0x55,
		/// Namespace facility code.
		Namespace = 0x4E,
		/// Transfer facility code.
		Transfer = 0x54
	};
}}
