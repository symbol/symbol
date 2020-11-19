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

#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult {
	namespace timesync {
		struct TimeSynchronizationConfiguration;
		class TimeSynchronizationState;
	}
}

namespace catapult { namespace timesync {

	/// Creates a registrar for a time synchronization service around \a timeSyncConfig and \a pTimeSyncState.
	/// \note This service is responsible for synchronizing the network time among nodes.
	DECLARE_SERVICE_REGISTRAR(TimeSynchronization)(
			const TimeSynchronizationConfiguration& timeSyncConfig,
			const std::shared_ptr<TimeSynchronizationState>& pTimeSyncState);
}}
