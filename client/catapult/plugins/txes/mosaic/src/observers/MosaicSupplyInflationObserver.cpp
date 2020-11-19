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

#include "Observers.h"
#include "src/cache/MosaicCache.h"
#include "catapult/model/InflationCalculator.h"

namespace catapult { namespace observers {

	using Notification = model::BlockNotification;

	DECLARE_OBSERVER(MosaicSupplyInflation, Notification)(MosaicId currencyMosaicId, const model::InflationCalculator& calculator) {
		return MAKE_OBSERVER(MosaicSupplyInflation, Notification, ([currencyMosaicId, calculator](
				const Notification&,
				ObserverContext& context) {
			auto inflationAmount = calculator.getSpotAmount(context.Height);
			if (Amount(0) == inflationAmount)
				return;

			// only supply needs to be updated here because HarvestFeeObserver credits/debits harvester
			auto& mosaicCache = context.Cache.sub<cache::MosaicCache>();
			auto mosaicIter = mosaicCache.find(currencyMosaicId);
			auto& mosaicEntry = mosaicIter.get();
			if (NotifyMode::Commit == context.Mode)
				mosaicEntry.increaseSupply(inflationAmount);
			else
				mosaicEntry.decreaseSupply(inflationAmount);
		}));
	}
}}
