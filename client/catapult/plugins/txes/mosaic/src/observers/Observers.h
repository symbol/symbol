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
#include "src/model/MosaicNotifications.h"
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace model { class InflationCalculator; } }

namespace catapult { namespace observers {

	/// Observes changes triggered by mosaic definition notifications and:
	/// - creates mosaics
	DECLARE_OBSERVER(MosaicDefinition, model::MosaicDefinitionNotification)();

	/// Observes changes triggered by mosaic supply change notifications and:
	/// - increases or decreases supply
	DECLARE_OBSERVER(MosaicSupplyChange, model::MosaicSupplyChangeNotification)();

	/// Observes block notifications and:
	/// - increases or decreases the supply of the currency mosaic (\a currencyMosaicId) given the inflation \a calculator
	DECLARE_OBSERVER(MosaicSupplyInflation, model::BlockNotification)(
			MosaicId currencyMosaicId,
			const model::InflationCalculator& calculator);
}}
