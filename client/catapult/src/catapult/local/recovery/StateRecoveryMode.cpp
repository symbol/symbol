/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "StateRecoveryMode.h"
#include "catapult/config/NodeConfiguration.h"

namespace catapult { namespace local {

	StateRecoveryMode CalculateStateRecoveryMode(const config::NodeConfiguration& config, const extensions::StateHeights& heights) {
		if (heights.Cache == heights.Storage)
			return StateRecoveryMode::None;

		if (Height(1) == heights.Cache)
			return StateRecoveryMode::Reseed;

		if (heights.Cache > heights.Storage) {
			std::ostringstream out;
			out << "cache height (" << heights.Cache << ")" << " is greater than storage height (" << heights.Storage << ")";
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		return config.EnableCacheDatabaseStorage ? StateRecoveryMode::None : StateRecoveryMode::Repair;
	}
}}
