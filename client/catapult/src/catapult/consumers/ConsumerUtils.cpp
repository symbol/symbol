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

#include "ConsumerUtils.h"
#include "ConsumerResults.h"
#include "catapult/disruptor/ConsumerInput.h"
#include "catapult/ionet/NodeInteractionResult.h"

namespace catapult { namespace consumers {

	ionet::NodeInteractionResult ToNodeInteractionResult(
			const model::NodeIdentity& sourceIdentity,
			const disruptor::ConsumerCompletionResult& result) {
		ionet::NodeInteractionResultCode code;
		auto validationResult = static_cast<validators::ValidationResult>(result.CompletionCode);
		switch (validators::GetSeverity(validationResult)) {
		case validators::ResultSeverity::Success:
			code = ionet::NodeInteractionResultCode::Success;
			break;
		case validators::ResultSeverity::Neutral:
			code = ionet::NodeInteractionResultCode::Neutral;
			break;
		default:
			code = ionet::NodeInteractionResultCode::Failure;
		}

		return ionet::NodeInteractionResult(sourceIdentity, code);
	}
}}
