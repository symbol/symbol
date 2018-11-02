/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "src/model/PropertyNotifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	/// Observes changes triggered by modify address property value notifications and:
	/// - adds / deletes property address value to / from cache
	DECLARE_OBSERVER(AddressPropertyValueModification, model::ModifyAddressPropertyValueNotification)();

	/// Observes changes triggered by modify mosaic property value notifications and:
	/// - adds / deletes property mosaic value to / from cache
	DECLARE_OBSERVER(MosaicPropertyValueModification, model::ModifyMosaicPropertyValueNotification)();

	/// Observes changes triggered by modify transaction type property value notifications and:
	/// - adds / deletes property transaction type value to / from cache
	DECLARE_OBSERVER(TransactionTypePropertyValueModification, model::ModifyTransactionTypePropertyValueNotification)();
}}
