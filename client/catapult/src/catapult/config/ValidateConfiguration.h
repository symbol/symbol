#pragma once

namespace catapult { namespace config { class LocalNodeConfiguration; } }

namespace catapult { namespace config {

	/// Validates \a config and throws an exception if it is invalid.
	void ValidateConfiguration(const LocalNodeConfiguration& config);
}}
