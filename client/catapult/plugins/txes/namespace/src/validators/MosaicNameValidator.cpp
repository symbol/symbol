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
#include "src/model/IdGenerator.h"
#include "src/model/NameChecker.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicNameNotification;

	DECLARE_STATELESS_VALIDATOR(MosaicName, Notification)(uint8_t maxNameSize) {
		return MAKE_STATELESS_VALIDATOR(MosaicName, [maxNameSize](const auto& notification) {
			if (MosaicId() == notification.MosaicId)
				return Failure_Mosaic_Name_Reserved;

			if (maxNameSize < notification.NameSize || !model::IsValidName(notification.NamePtr, notification.NameSize))
				return Failure_Mosaic_Invalid_Name;

			auto name = utils::RawString(reinterpret_cast<const char*>(notification.NamePtr), notification.NameSize);
			if (notification.MosaicId != model::GenerateMosaicId(notification.ParentId, name))
				return Failure_Mosaic_Name_Id_Mismatch;

			return ValidationResult::Success;
		});
	}
}}
