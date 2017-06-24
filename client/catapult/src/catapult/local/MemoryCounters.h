#pragma once
#include <vector>

namespace catapult { namespace utils { class DiagnosticCounter; } }

namespace catapult { namespace local {

	/// Adds process memory counters to \a counters.
	void AddMemoryCounters(std::vector<utils::DiagnosticCounter>& counters);
}}
