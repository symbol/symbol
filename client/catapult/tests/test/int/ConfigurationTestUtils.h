#pragma once
#include "LocalNodeTestUtils.h"
#include <string>

namespace catapult { namespace test {

	/// Prepares the configuration by copying it into the \a destination directory and setting
	/// the isHarvestingEnabled flag according to \a nodeFlag.
	void PrepareConfiguration(const std::string& destination, NodeFlag nodeFlag);
}}
