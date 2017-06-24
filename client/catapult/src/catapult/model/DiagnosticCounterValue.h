#pragma once
#include <stdint.h>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// A diagnostic counter value.
	struct DiagnosticCounterValue {
		/// The counter id.
		uint64_t Id;

		/// The counter value.
		uint64_t Value;
	};

#pragma pack(pop)
}}
