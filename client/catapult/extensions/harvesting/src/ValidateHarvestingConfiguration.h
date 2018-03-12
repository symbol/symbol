#pragma once

namespace catapult { namespace harvesting { struct HarvestingConfiguration; } }

namespace catapult { namespace harvesting {

	/// Validates \a config and throws an exception if it is invalid.
	void ValidateHarvestingConfiguration(const HarvestingConfiguration& config);
}}
