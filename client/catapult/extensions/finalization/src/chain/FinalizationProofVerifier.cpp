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

#include "FinalizationProofVerifier.h"
#include "RoundContext.h"
#include "RoundMessageAggregator.h"
#include "finalization/src/model/FinalizationContext.h"
#include "finalization/src/model/FinalizationMessage.h"
#include "finalization/src/model/FinalizationProof.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace chain {

#define DEFINE_ENUM VerifyFinalizationProofResult
#define ENUM_LIST VERIFY_FINALIZATION_PROOF_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	namespace {
		std::shared_ptr<model::FinalizationMessage> CreateTemplateMessage(
				FinalizationPoint point,
				const model::FinalizationMessageGroup& messageGroup) {
			uint32_t hashesPayloadSize = static_cast<uint32_t>(messageGroup.HashesCount * Hash256::Size);
			uint32_t size = SizeOf32<model::FinalizationMessage>() + hashesPayloadSize;

			auto pMessage = utils::MakeSharedWithSize<model::FinalizationMessage>(size);
			pMessage->Size = size;
			pMessage->HashesCount = messageGroup.HashesCount;
			pMessage->StepIdentifier = { point, messageGroup.Stage };
			pMessage->Height = messageGroup.Height;

			std::memcpy(reinterpret_cast<void*>(pMessage->HashesPtr()), messageGroup.HashesPtr(), hashesPayloadSize);
			return pMessage;
		}
	}

	VerifyFinalizationProofResult VerifyFinalizationProof(
			const model::FinalizationProof& proof,
			const model::FinalizationContext& context) {
		if (model::FinalizationProofHeader::Current_Version != proof.Version)
			return VerifyFinalizationProofResult::Failure_Invalid_Version;

		if (proof.Point != context.point())
			return VerifyFinalizationProofResult::Failure_Invalid_Point;

		auto pMessageAggregator = CreateRoundMessageAggregator(context);
		for (const auto& messageGroup : proof.MessageGroups()) {
			auto pTemplateMessage = CreateTemplateMessage(proof.Point, messageGroup);
			for (auto i = 0u; i < messageGroup.SignaturesCount; ++i) {
				pTemplateMessage->Signature = messageGroup.SignaturesPtr()[i];
				auto addResult = pMessageAggregator->add(pTemplateMessage);
				if (addResult <= chain::RoundMessageAggregatorAddResult::Neutral_Redundant) {
					CATAPULT_LOG(warning) << "finalization message for proof " << proof.Hash << " rejected due to " << addResult;
					return VerifyFinalizationProofResult::Failure_Invalid_Messsage;
				}
			}
		}

		auto bestPrecommitResultPair = pMessageAggregator->roundContext().tryFindBestPrecommit();
		if (!bestPrecommitResultPair.second)
			return VerifyFinalizationProofResult::Failure_No_Precommit;

		if (bestPrecommitResultPair.first.Height != proof.Height)
			return VerifyFinalizationProofResult::Failure_Invalid_Height;

		if (bestPrecommitResultPair.first.Hash != proof.Hash)
			return VerifyFinalizationProofResult::Failure_Invalid_Hash;

		return VerifyFinalizationProofResult::Success;
	}
}}
