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

#include "Validators.h"

namespace catapult { namespace validators {

	using Notification = model::TransferMosaicsNotification;

	DEFINE_STATELESS_VALIDATOR(TransferMosaics, [](const Notification& notification) {
		// check strict ordering of mosaics
		if (1 >= notification.MosaicsCount)
			return ValidationResult::Success;

		auto pMosaics = notification.MosaicsPtr;
		auto lastMosaicId = pMosaics[0].MosaicId;
		for (auto i = 1u; i < notification.MosaicsCount; ++i) {
			auto currentMosaicId = pMosaics[i].MosaicId;
			if (lastMosaicId >= currentMosaicId)
				return Failure_Transfer_Out_Of_Order_Mosaics;

			lastMosaicId = currentMosaicId;
		}

		return ValidationResult::Success;
	})
}}
