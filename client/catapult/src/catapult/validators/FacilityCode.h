#pragma once
#include <stdint.h>

namespace catapult { namespace validators {

	/// Possible facility codes.
	enum class FacilityCode : uint8_t {
		/// Aggregate facility code.
		Aggregate = 0x41,
		/// Chain facility code.
		Chain = 0xFF,
		/// Consumer facility code.
		Consumer = 0xFE,
		/// Core facility code.
		Core = 0x43,
		/// Hash facility code.
		Hash = 0x48,
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
