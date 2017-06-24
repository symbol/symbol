#pragma once
#include <stdint.h>

namespace catapult { namespace model {

	/// Bitwise artifact info attributes.
	enum class ArtifactInfoAttributes : uint8_t {
		/// No flags are set.
		None = 0,
		/// Flag that is set to indicate a known artifact.
		Is_Known = 1
	};
}}
