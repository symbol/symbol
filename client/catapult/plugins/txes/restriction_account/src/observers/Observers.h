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
#include "src/model/AccountRestrictionNotifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	/// Observes changes triggered by account address restriction value modification notifications and:
	/// - adds / deletes account address restriction value to / from cache
	/// - indicates compact serialization after \a compactFormatForkHeight
	DECLARE_OBSERVER(AccountAddressRestrictionValueModification, model::ModifyAccountAddressRestrictionValueNotification)(
			Height compactFormatForkHeight);

	/// Observes changes triggered by account mosaic restriction value modification notifications and:
	/// - adds / deletes account mosaic restriction value to / from cache
	/// - indicates compact serialization after \a compactFormatForkHeight
	DECLARE_OBSERVER(AccountMosaicRestrictionValueModification, model::ModifyAccountMosaicRestrictionValueNotification)(
			Height compactFormatForkHeight);

	/// Observes changes triggered by account operation restriction value modification notifications and:
	/// - adds / deletes account operation restriction value to / from cache
	/// - indicates compact serialization after \a compactFormatForkHeight
	DECLARE_OBSERVER(AccountOperationRestrictionValueModification, model::ModifyAccountOperationRestrictionValueNotification)(
			Height compactFormatForkHeight);
}}
