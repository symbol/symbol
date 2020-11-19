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
#include "src/model/MosaicRestrictionNotifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	/// Observes changes triggered by new value notifications of mosaic global restriction modifications and:
	/// - adds / deletes mosaic global restriction value to / from cache (only executes during commit)
	DECLARE_OBSERVER(MosaicGlobalRestrictionCommitModification, model::MosaicGlobalRestrictionModificationNewValueNotification)();

	/// Observes changes triggered by previous value notifications of mosaic global restriction modifications and:
	/// - adds / deletes mosaic global restriction value to / from cache (only executes during rollback)
	DECLARE_OBSERVER(MosaicGlobalRestrictionRollbackModification, model::MosaicGlobalRestrictionModificationPreviousValueNotification)();

	/// Observes changes triggered by new value notifications of mosaic address restriction modifications and:
	/// - adds / deletes mosaic address restriction value to / from cache (only executes during commit)
	DECLARE_OBSERVER(MosaicAddressRestrictionCommitModification, model::MosaicAddressRestrictionModificationNewValueNotification)();

	/// Observes changes triggered by previous value notifications of mosaic address restriction modifications and:
	/// - adds / deletes mosaic address restriction value to / from cache (only executes during rollback)
	DECLARE_OBSERVER(MosaicAddressRestrictionRollbackModification, model::MosaicAddressRestrictionModificationPreviousValueNotification)();
}}
