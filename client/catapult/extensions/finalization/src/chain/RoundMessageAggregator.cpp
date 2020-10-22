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

#include "RoundMessageAggregator.h"
#include "RoundContext.h"
#include "finalization/src/model/FinalizationContext.h"
#include "finalization/src/model/FinalizationMessage.h"
#include "finalization/src/model/VotingSet.h"
#include "catapult/model/HeightGrouping.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"
#include <unordered_map>

namespace catapult { namespace chain {

#define DEFINE_ENUM RoundMessageAggregatorAddResult
#define ENUM_LIST ROUND_MESSAGE_AGGREGATOR_ADD_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	namespace {
		// region utils

		using MessageKey = std::pair<VotingKey, bool>;

		struct MessageDescriptor {
			std::shared_ptr<const model::FinalizationMessage> pMessage;
			Hash256 Hash;
			utils::ShortHash ShortHash;
		};

		MessageDescriptor CreateMessageDescriptor(const std::shared_ptr<const model::FinalizationMessage>& pMessage) {
			MessageDescriptor descriptor;
			descriptor.pMessage = pMessage;
			descriptor.Hash = model::CalculateMessageHash(*descriptor.pMessage);
			descriptor.ShortHash = utils::ToShortHash(descriptor.Hash);
			return descriptor;
		}

		struct MessageKeyHasher {
			size_t operator()(const MessageKey& pair) const {
				return utils::ArrayHasher<VotingKey>()(pair.first);
			}
		};

		uint64_t CalculateWeightedThreshold(const model::FinalizationContext& finalizationContext) {
			return finalizationContext.weight().unwrap() * finalizationContext.config().Threshold / finalizationContext.config().Size;
		}

		bool IsPrevote(const model::FinalizationMessage& message) {
			return model::FinalizationStage::Prevote == message.Data().StepIdentifier.Stage();
		}

		// endregion

		// region DefaultRoundMessageAggregator

		class DefaultRoundMessageAggregator : public RoundMessageAggregator {
		public:
			explicit DefaultRoundMessageAggregator(const model::FinalizationContext& finalizationContext)
					: m_finalizationContext(finalizationContext)
					, m_maxResponseSize(m_finalizationContext.config().MessageSynchronizationMaxResponseSize.bytes())
					, m_roundContext(m_finalizationContext.weight().unwrap(), CalculateWeightedThreshold(m_finalizationContext))
			{}

		public:
			size_t size() const override {
				return m_messages.size();
			}

			const model::FinalizationContext& finalizationContext() const override {
				return m_finalizationContext;
			}

			const RoundContext& roundContext() const override {
				return m_roundContext;
			}

			model::ShortHashRange shortHashes() const override {
				auto shortHashes = model::EntityRange<utils::ShortHash>::PrepareFixed(m_messages.size());
				auto shortHashesIter = shortHashes.begin();
				for (const auto& messagePair : m_messages)
					*shortHashesIter++ = messagePair.second.ShortHash;

				return shortHashes;
			}

			RoundMessageAggregator::UnknownMessages unknownMessages(const utils::ShortHashesSet& knownShortHashes) const override {
				uint64_t totalSize = 0;
				UnknownMessages messages;
				for (const auto& messagePair : m_messages) {
					const auto& pMessage = messagePair.second.pMessage;
					auto iter = knownShortHashes.find(messagePair.second.ShortHash);
					if (knownShortHashes.cend() == iter) {
						totalSize += pMessage->Size;
						if (totalSize > m_maxResponseSize)
							return messages;

						messages.push_back(pMessage);
					}
				}

				return messages;
			}

		public:
			RoundMessageAggregatorAddResult add(const std::shared_ptr<model::FinalizationMessage>& pMessage) override {
				auto maxHashesPerPoint = m_finalizationContext.config().MaxHashesPerPoint;
				CATAPULT_LOG(trace)
						<< "received message at " << pMessage->Data().StepIdentifier
						<< " with " << pMessage->Data().HashesCount << " hashes (max " << maxHashesPerPoint << ")";

				if (0 == pMessage->Data().HashesCount || pMessage->Data().HashesCount > maxHashesPerPoint)
					return RoundMessageAggregatorAddResult::Failure_Invalid_Hashes;

				auto isPrevote = IsPrevote(*pMessage);
				auto lastHashHeight = pMessage->Data().Height + Height(pMessage->Data().HashesCount - 1);

				// only consider messages that have at least one hash at or after the epoch height
				if (m_finalizationContext.height() > lastHashHeight)
					return RoundMessageAggregatorAddResult::Failure_Invalid_Height;

				if (isPrevote) {
					auto votingSetGrouping = m_finalizationContext.config().VotingSetGrouping;
					auto previousEpoch = pMessage->Data().StepIdentifier.Epoch - FinalizationEpoch(1);
					auto epochMinHeight = model::CalculateVotingSetEndHeight(previousEpoch, votingSetGrouping);
					auto epochMaxHeight = model::CalculateVotingSetEndHeight(pMessage->Data().StepIdentifier.Epoch, votingSetGrouping);

					if (pMessage->Data().Height < epochMinHeight || lastHashHeight > epochMaxHeight)
						return RoundMessageAggregatorAddResult::Failure_Invalid_Hashes;
				} else {
					if (1 != pMessage->Data().HashesCount)
						return RoundMessageAggregatorAddResult::Failure_Invalid_Hashes;
				}

				auto messageKey = std::make_pair(pMessage->Signature().Root.ParentPublicKey, isPrevote);
				auto messageIter = m_messages.find(messageKey);
				if (m_messages.cend() != messageIter) {
					return messageIter->second.Hash == model::CalculateMessageHash(*pMessage)
							? RoundMessageAggregatorAddResult::Neutral_Redundant
							: RoundMessageAggregatorAddResult::Failure_Conflicting;
				}

				auto processResultPair = model::ProcessMessage(*pMessage, m_finalizationContext);
				if (model::ProcessMessageResult::Success != processResultPair.first) {
					CATAPULT_LOG(warning) << "rejecting finalization message with result " << processResultPair.first;
					return RoundMessageAggregatorAddResult::Failure_Processing;
				}

				CATAPULT_LOG(trace)
						<< "processing message for epoch " << m_finalizationContext.epoch() << " with weight " << processResultPair.second
						<< std::endl << *pMessage;

				m_messages.emplace(messageKey, CreateMessageDescriptor(pMessage));

				if (isPrevote) {
					m_roundContext.acceptPrevote(
							pMessage->Data().Height,
							pMessage->HashesPtr(),
							pMessage->Data().HashesCount,
							processResultPair.second);
					return RoundMessageAggregatorAddResult::Success_Prevote;
				} else {
					m_roundContext.acceptPrecommit(pMessage->Data().Height, *pMessage->HashesPtr(), processResultPair.second);
					return RoundMessageAggregatorAddResult::Success_Precommit;
				}
			}

		private:
			model::FinalizationContext m_finalizationContext;
			uint64_t m_maxResponseSize;
			chain::RoundContext m_roundContext;
			std::unordered_map<MessageKey, MessageDescriptor, MessageKeyHasher> m_messages;
		};

		// endregion
	}

	std::unique_ptr<RoundMessageAggregator> CreateRoundMessageAggregator(const model::FinalizationContext& finalizationContext) {
		return std::make_unique<DefaultRoundMessageAggregator>(finalizationContext);
	}
}}
