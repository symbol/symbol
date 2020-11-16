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

#include "Validators.h"
#include "catapult/model/BlockUtils.h"

namespace catapult { namespace validators {

	using Notification = model::BlockTypeNotification;

	namespace {
		model::EntityType CalculateExpectedBlockType(Height height, uint64_t importanceGrouping, Height forkHeight) {
			if (Height(1) == height)
				return model::Entity_Type_Block_Nemesis;

			if (height <= forkHeight)
				return model::Entity_Type_Block_Normal;

			return model::CalculateBlockTypeFromHeight(height, importanceGrouping);
		}
	}

	DECLARE_STATELESS_VALIDATOR(BlockType, Notification)(uint64_t importanceGrouping, Height forkHeight) {
		return MAKE_STATELESS_VALIDATOR(BlockType, ([importanceGrouping, forkHeight](const Notification& notification) {
			auto expectedBlockType = CalculateExpectedBlockType(notification.BlockHeight, importanceGrouping, forkHeight);
			return expectedBlockType == notification.BlockType ? ValidationResult::Success : Failure_Core_Unexpected_Block_Type;
		}));
	}
}}
