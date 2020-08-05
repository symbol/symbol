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

#include "FinalizationMessage.h"
#include "FinalizationContext.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto_voting/OtsTree.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace model {

#define DEFINE_ENUM ProcessMessageResult
#define ENUM_LIST PROCESS_MESSAGE_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	namespace {
		RawBuffer ToBuffer(const FinalizationMessage& message) {
			return {
				reinterpret_cast<const uint8_t*>(&message) + FinalizationMessage::Header_Size,
				message.Size - FinalizationMessage::Header_Size
			};
		}
	}

	Hash256 CalculateMessageHash(const FinalizationMessage& message) {
		Hash256 messageHash;
		crypto::Sha3_256(ToBuffer(message), messageHash);
		return messageHash;
	}

	// TODO: FinalizationContext::lookup expects BLS key, but, for now, interpret it as ED25519 key

	std::unique_ptr<FinalizationMessage> PrepareMessage(
			crypto::OtsTree& otsTree,
			const crypto::StepIdentifier& stepIdentifier,
			Height height,
			const HashRange& hashes,
			const FinalizationContext& context) {
		auto accountView = context.lookup(otsTree.rootPublicKey().copyTo<VotingKey>());
		if (Amount() == accountView.Weight)
			return nullptr;

		// 1. create message and copy hashes
		auto numHashes = static_cast<uint32_t>(hashes.size());
		uint32_t messageSize = SizeOf32<FinalizationMessage>() + numHashes * static_cast<uint32_t>(Hash256::Size);

		auto pMessage = utils::MakeUniqueWithSize<FinalizationMessage>(messageSize);
		pMessage->Size = messageSize;
		pMessage->HashesCount = numHashes;
		pMessage->StepIdentifier = stepIdentifier;
		pMessage->Height = height;

		auto* pHash = pMessage->HashesPtr();
		for (const auto& hash : hashes)
			*pHash++ = hash;

		// 2. sign
		pMessage->Signature = otsTree.sign(pMessage->StepIdentifier, ToBuffer(*pMessage));
		return pMessage;
	}

	std::pair<ProcessMessageResult, size_t> ProcessMessage(const FinalizationMessage& message, const FinalizationContext& context) {
		auto accountView = context.lookup(message.Signature.Root.ParentPublicKey.copyTo<VotingKey>());
		if (Amount() == accountView.Weight)
			return std::make_pair(ProcessMessageResult::Failure_Voter, 0);

		if (!crypto::Verify(message.Signature, message.StepIdentifier, ToBuffer(message)))
			return std::make_pair(ProcessMessageResult::Failure_Message_Signature, 0);

		return std::make_pair(ProcessMessageResult::Success, accountView.Weight.unwrap());
	}
}}
