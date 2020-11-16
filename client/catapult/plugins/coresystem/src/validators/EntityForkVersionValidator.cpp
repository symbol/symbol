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
#include "src/model/VotingKeyLinkTransaction.h"
#include "catapult/model/BlockChainConfiguration.h"

namespace catapult { namespace validators {

	using Notification = model::EntityNotification;

	namespace {
		ValidationResult CheckInflection(Height forkHeight, uint8_t preVersion, uint8_t postVersion, Height height, uint8_t version) {
			return (height <= forkHeight && preVersion == version) || (height > forkHeight && postVersion == version)
					? ValidationResult::Success
					: Failure_Core_Invalid_Version;
		}
	}

	DECLARE_STATEFUL_VALIDATOR(EntityForkVersion, Notification)(const model::BlockChainForkHeights& forkHeights) {
		return MAKE_STATEFUL_VALIDATOR(EntityForkVersion, [forkHeights](
				const Notification& notification,
				const ValidatorContext& context) {
			auto entityType = notification.EntityType;
			if (model::BasicEntityType::Block == model::ToBasicEntityType(entityType))
				return CheckInflection(forkHeights.ImportanceBlock, 1, 2, context.Height, notification.EntityVersion);

			if (model::Entity_Type_Voting_Key_Link == notification.EntityType)
				return CheckInflection(forkHeights.VotingKeyLinkV2, 1, 2, context.Height, notification.EntityVersion);

			return ValidationResult::Success;
		});
	}
}}
