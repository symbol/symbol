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

#include "FinalizationProofUtils.h"
#include <map>

namespace catapult { namespace model {

	namespace {
		using ConstMessages = std::vector<std::shared_ptr<const FinalizationMessage>>;
		using MutableMessageGroups = std::vector<std::unique_ptr<FinalizationMessageGroup>>;

		const uint8_t* GetVerifiableDataStart(const FinalizationMessage& message) {
			return reinterpret_cast<const uint8_t*>(&message) + FinalizationMessage::Header_Size;
		}

		struct GroupFinalizationMessageComparer {
			bool operator()(const std::shared_ptr<const FinalizationMessage>& pLhs, const std::shared_ptr<const FinalizationMessage>& pRhs)
					const {
				if (pLhs->Size != pRhs->Size)
					return pLhs->Size < pRhs->Size;

				auto numBytesToCompare = pLhs->Size - FinalizationMessage::Header_Size;
				return std::memcmp(GetVerifiableDataStart(*pLhs), GetVerifiableDataStart(*pRhs), numBytesToCompare) < 0;
			}
		};

		using MessageSignatureGroups = std::
				map<std::shared_ptr<const FinalizationMessage>, std::vector<crypto::BmTreeSignature>, GroupFinalizationMessageComparer>;

		MutableMessageGroups GroupMessages(const model::FinalizationRound& round, const ConstMessages& messages) {
			MessageSignatureGroups messageSignatureGroups;
			for (const auto& pMessage : messages) {
				auto checkEqual = [&round, &message = *pMessage](const auto& expected, const auto& actual, const auto* description) {
					if (expected == actual)
						return true;

					CATAPULT_LOG(warning) << "skipping message with unexpected " << description << " " << actual << " from "
										  << message.Signature.Root.ParentPublicKey << " when grouping messages at round " << round;
					return false;
				};

				auto canGroup = checkEqual(round, pMessage->StepIdentifier.Round(), "round")
								&& checkEqual(0u, pMessage->FinalizationMessage_Reserved1, "padding")
								&& checkEqual(FinalizationMessage::Current_Version, pMessage->Version, "version");
				if (!canGroup)
					continue;

				auto iter = messageSignatureGroups.find(pMessage);
				if (messageSignatureGroups.cend() == iter)
					iter = messageSignatureGroups.emplace(pMessage, std::vector<crypto::BmTreeSignature>()).first;

				iter->second.push_back(pMessage->Signature);
			}

			MutableMessageGroups messageGroups;
			for (const auto& messageSignatureGroupPair : messageSignatureGroups) {
				auto numSignatures = static_cast<uint32_t>(messageSignatureGroupPair.second.size());
				const auto& pTemplateMessage = messageSignatureGroupPair.first;

				uint32_t hashesPayloadSize = static_cast<uint32_t>(pTemplateMessage->HashesCount * Hash256::Size);
				uint32_t signaturesPayloadSize = numSignatures * SizeOf32<crypto::BmTreeSignature>();
				uint32_t size = SizeOf32<FinalizationMessageGroup>() + hashesPayloadSize + signaturesPayloadSize;
				auto pMessageGroup = utils::MakeUniqueWithSize<FinalizationMessageGroup>(size);
				pMessageGroup->Size = size;
				pMessageGroup->HashesCount = pTemplateMessage->HashesCount;
				pMessageGroup->SignaturesCount = numSignatures;
				pMessageGroup->Stage = pTemplateMessage->StepIdentifier.Stage();
				pMessageGroup->Height = pTemplateMessage->Height;

				std::memcpy(reinterpret_cast<void*>(pMessageGroup->HashesPtr()), pTemplateMessage->HashesPtr(), hashesPayloadSize);
				std::memcpy(
						reinterpret_cast<void*>(pMessageGroup->SignaturesPtr()),
						messageSignatureGroupPair.second.data(),
						signaturesPayloadSize);

				messageGroups.push_back(std::move(pMessageGroup));
			}

			return messageGroups;
		}

		void CopyMessageGroups(uint8_t* pDestination, const MutableMessageGroups& messageGroups) {
			for (auto i = 0u; i < messageGroups.size(); ++i) {
				const auto& pMessageGroup = messageGroups[i];
				std::memcpy(pDestination, pMessageGroup.get(), pMessageGroup->Size);
				pDestination += pMessageGroup->Size;
			}
		}

		uint32_t CalculateTotalSize(const MutableMessageGroups& messageGroups) {
			uint32_t totalMessageGroupsSize = 0;
			for (const auto& pMessageGroup : messageGroups)
				totalMessageGroupsSize += pMessageGroup->Size;

			return totalMessageGroupsSize;
		}

		std::unique_ptr<FinalizationProof> GenerateProofWithMessageGroups(const MutableMessageGroups& messageGroups) {
			uint32_t size = SizeOf32<FinalizationProofHeader>() + CalculateTotalSize(messageGroups);
			auto pProof = utils::MakeUniqueWithSize<FinalizationProof>(size);
			std::memset(static_cast<void*>(pProof.get()), 0, sizeof(FinalizationProofHeader));
			pProof->Size = size;
			pProof->Version = FinalizationProofHeader::Current_Version;

			// append all the message groups
			auto* pDestination = reinterpret_cast<uint8_t*>(pProof->MessageGroupsPtr());
			CopyMessageGroups(pDestination, messageGroups);
			return pProof;
		}
	}

	std::unique_ptr<FinalizationProof> CreateFinalizationProof(const FinalizationStatistics& statistics, const ConstMessages& messages) {
		auto pProof = GenerateProofWithMessageGroups(GroupMessages(statistics.Round, messages));
		pProof->Round = statistics.Round;
		pProof->Height = statistics.Height;
		pProof->Hash = statistics.Hash;
		return pProof;
	}
}}
