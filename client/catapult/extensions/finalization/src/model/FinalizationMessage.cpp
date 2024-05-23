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

#include "FinalizationMessage.h"
#include "FinalizationContext.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto_voting/AggregateBmPrivateKeyTree.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult {
namespace model {

#define DEFINE_ENUM ProcessMessageResult
#define ENUM_LIST PROCESS_MESSAGE_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	std::ostream& operator<<(std::ostream& out, const FinalizationMessage& message) {
		out << "message for " << message.StepIdentifier << " at " << message.Height << " from " << message.Signature.Root.ParentPublicKey;

		for (auto i = 0u; i < message.HashesCount; ++i)
			out << std::endl
				<< " + " << message.HashesPtr()[i] << " @ " << message.Height + Height(i);

		return out;
	}

	namespace {
		RawBuffer ToBuffer(const FinalizationMessage& message) {
			return { reinterpret_cast<const uint8_t*>(&message) + FinalizationMessage::Header_Size,
				message.Size - FinalizationMessage::Header_Size };
		}
	}

	Hash256 CalculateMessageHash(const FinalizationMessage& message) {
		Hash256 messageHash;
		crypto::Sha3_256({ reinterpret_cast<const uint8_t*>(&message), message.Size }, messageHash);
		return messageHash;
	}

	std::unique_ptr<FinalizationMessage> PrepareMessage(
		crypto::AggregateBmPrivateKeyTree& bmPrivateKeyTree,
		const StepIdentifier& stepIdentifier,
		Height height,
		const HashRange& hashes) {
		// 1. check if message can be signed
		auto keyIdentifier = StepIdentifierToBmKeyIdentifier(stepIdentifier);
		if (!bmPrivateKeyTree.canSign(keyIdentifier))
			return nullptr;

		// 2. create message and copy hashes
		auto numHashes = static_cast<uint32_t>(hashes.size());
		uint32_t messageSize = SizeOf32<FinalizationMessage>() + numHashes * static_cast<uint32_t>(Hash256::Size);

		auto pMessage = utils::MakeUniqueWithSize<FinalizationMessage>(messageSize);
		pMessage->Size = messageSize;
		pMessage->FinalizationMessage_Reserved1 = 0;
		pMessage->Version = FinalizationMessage::Current_Version;
		pMessage->HashesCount = numHashes;
		pMessage->StepIdentifier = stepIdentifier;
		pMessage->Height = height;

		auto* pHash = pMessage->HashesPtr();
		for (const auto& hash : hashes)
			*pHash++ = hash;

		// 3. sign
		pMessage->Signature = bmPrivateKeyTree.sign(keyIdentifier, ToBuffer(*pMessage));
		return pMessage;
	}

	std::pair<ProcessMessageResult, size_t> ProcessMessage(const FinalizationMessage& message, const FinalizationContext& context) {
		auto accountView = context.lookup(message.Signature.Root.ParentPublicKey);
		if (Amount() == accountView.Weight)
			return std::make_pair(ProcessMessageResult::Failure_Voter, 0);

		if (0 != message.FinalizationMessage_Reserved1)
			return std::make_pair(ProcessMessageResult::Failure_Padding, 0);

		if (FinalizationMessage::Current_Version != message.Version)
			return std::make_pair(ProcessMessageResult::Failure_Version, 0);

		auto keyIdentifier = StepIdentifierToBmKeyIdentifier(message.StepIdentifier);
		if (!crypto::Verify(message.Signature, keyIdentifier, ToBuffer(message)))
			return std::make_pair(ProcessMessageResult::Failure_Signature, 0);

		return std::make_pair(ProcessMessageResult::Success, accountView.Weight.unwrap());
	}
}
}
