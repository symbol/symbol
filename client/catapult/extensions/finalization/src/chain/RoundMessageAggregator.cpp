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

#include "RoundMessageAggregator.h"
#include "RoundContext.h"
#include "finalization/src/model/FinalizationContext.h"
#include "finalization/src/model/FinalizationMessage.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"
#include <unordered_map>

namespace catapult { namespace chain {

#define DEFINE_ENUM RoundMessageAggregatorAddResult
#define ENUM_LIST ROUND_MESSAGE_AGGREGATOR_ADD_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	// region RoundMessageAggregatorState

	namespace {
		using MessageKey = std::pair<Key, bool>;

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
				return utils::ArrayHasher<Key>()(pair.first);
			}
		};

		uint64_t CalculateWeightedThreshold(const model::FinalizationContext& finalizationContext) {
			return finalizationContext.weight().unwrap() * finalizationContext.config().Threshold / finalizationContext.config().Size;
		}
	}

	struct RoundMessageAggregatorState {
	public:
		RoundMessageAggregatorState(uint64_t maxResponseSize, const model::FinalizationContext& finalizationContext)
				: MaxResponseSize(maxResponseSize)
				, FinalizationContext(finalizationContext)
				, RoundContext(FinalizationContext.weight().unwrap(), CalculateWeightedThreshold(FinalizationContext))
		{}

	public:
		uint64_t MaxResponseSize;
		model::FinalizationContext FinalizationContext; // TODO: review if this should be reference after higher layer is written
		chain::RoundContext RoundContext;
		std::unordered_map<MessageKey, MessageDescriptor, MessageKeyHasher> Messages;
	};

	// endregion

	// region RoundMessageAggregatorView

	RoundMessageAggregatorView::RoundMessageAggregatorView(
			const RoundMessageAggregatorState& state,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_state(state)
			, m_readLock(std::move(readLock))
	{}

	size_t RoundMessageAggregatorView::size() const {
		return m_state.Messages.size();
	}

	const model::FinalizationContext& RoundMessageAggregatorView::finalizationContext() const {
		return m_state.FinalizationContext;
	}

	const RoundContext& RoundMessageAggregatorView::roundContext() const {
		return m_state.RoundContext;
	}

	model::ShortHashRange RoundMessageAggregatorView::shortHashes() const {
		auto shortHashes = model::EntityRange<utils::ShortHash>::PrepareFixed(m_state.Messages.size());
		auto shortHashesIter = shortHashes.begin();
		for (const auto& messagePair : m_state.Messages)
			*shortHashesIter++ = messagePair.second.ShortHash;

		return shortHashes;
	}

	RoundMessageAggregatorView::UnknownMessages RoundMessageAggregatorView::unknownMessages(
			FinalizationPoint point,
			const utils::ShortHashesSet& knownShortHashes) const {
		UnknownMessages messages;
		if (m_state.FinalizationContext.point() != point)
			return messages;

		uint64_t totalSize = 0;
		for (const auto& messagePair : m_state.Messages) {
			const auto& pMessage = messagePair.second.pMessage;
			auto iter = knownShortHashes.find(messagePair.second.ShortHash);
			if (knownShortHashes.cend() == iter) {
				totalSize += pMessage->Size;
				if (totalSize > m_state.MaxResponseSize)
					return messages;

				messages.push_back(pMessage);
			}
		}

		return messages;
	}

	// endregion

	// region RoundMessageAggregatorModifier

	namespace {
		constexpr bool IsPrevote(const model::FinalizationMessage& message) {
			return 1 == message.StepIdentifier.Round;
		}
	}

	RoundMessageAggregatorModifier::RoundMessageAggregatorModifier(
			RoundMessageAggregatorState& state,
			utils::SpinReaderWriterLock::WriterLockGuard&& writeLock)
			: m_state(state)
			, m_writeLock(std::move(writeLock))
	{}

	RoundMessageAggregatorAddResult RoundMessageAggregatorModifier::add(const std::shared_ptr<model::FinalizationMessage>& pMessage) {
		if (0 == pMessage->HashesCount || pMessage->HashesCount > m_state.FinalizationContext.config().MaxHashesPerPoint)
			return RoundMessageAggregatorAddResult::Failure_Invalid_Hashes;

		if (m_state.FinalizationContext.point() != FinalizationPoint(pMessage->StepIdentifier.Point))
			return RoundMessageAggregatorAddResult::Failure_Invalid_Point;

		auto isPrevote = IsPrevote(*pMessage);
		if (!isPrevote && 1 != pMessage->HashesCount)
			return RoundMessageAggregatorAddResult::Failure_Invalid_Hashes;

		// only consider messages that have at least one hash at or after the last finalized height
		if (m_state.FinalizationContext.height() > pMessage->Height + Height(pMessage->HashesCount - 1))
			return RoundMessageAggregatorAddResult::Failure_Invalid_Height;

		auto messageKey = std::make_pair(pMessage->Signature.Root.ParentPublicKey, isPrevote);
		auto messageIter = m_state.Messages.find(messageKey);
		if (m_state.Messages.cend() != messageIter) {
			return messageIter->second.Hash == model::CalculateMessageHash(*pMessage)
					? RoundMessageAggregatorAddResult::Neutral_Redundant
					: RoundMessageAggregatorAddResult::Failure_Conflicting;
		}

		auto processResultPair = model::ProcessMessage(*pMessage, m_state.FinalizationContext);
		if (model::ProcessMessageResult::Success != processResultPair.first) {
			CATAPULT_LOG(warning) << "rejecting finalization message with result " << processResultPair.first;
			return RoundMessageAggregatorAddResult::Failure_Processing;
		}

		m_state.Messages.emplace(messageKey, CreateMessageDescriptor(pMessage));

		if (isPrevote) {
			m_state.RoundContext.acceptPrevote(pMessage->Height, pMessage->HashesPtr(), pMessage->HashesCount, processResultPair.second);
			return RoundMessageAggregatorAddResult::Success_Prevote;
		} else {
			m_state.RoundContext.acceptPrecommit(pMessage->Height, *pMessage->HashesPtr(), processResultPair.second);
			return RoundMessageAggregatorAddResult::Success_Precommit;
		}
	}

	// endregion

	// region RoundMessageAggregator

	RoundMessageAggregator::RoundMessageAggregator(uint64_t maxResponseSize, const model::FinalizationContext& finalizationContext)
			: m_pState(std::make_unique<RoundMessageAggregatorState>(maxResponseSize, finalizationContext))
	{}

	RoundMessageAggregator::~RoundMessageAggregator() = default;

	RoundMessageAggregatorView RoundMessageAggregator::view() const {
		auto readLock = m_lock.acquireReader();
		return RoundMessageAggregatorView(*m_pState, std::move(readLock));
	}

	RoundMessageAggregatorModifier RoundMessageAggregator::modifier() {
		auto writeLock = m_lock.acquireWriter();
		return RoundMessageAggregatorModifier(*m_pState, std::move(writeLock));
	}

	// endregion
}}
