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

#include "MultiRoundMessageAggregator.h"
#include "RoundContext.h"
#include "finalization/src/model/FinalizationMessage.h"
#include "finalization/src/model/FinalizationRoundRange.h"

namespace catapult { namespace chain {

	// region MultiRoundMessageAggregatorState

	struct MultiRoundMessageAggregatorState {
	public:
		MultiRoundMessageAggregatorState(
				uint64_t maxResponseSize,
				const model::FinalizationRound& round,
				const model::HeightHashPair& previousFinalizedHeightHashPair,
				const MultiRoundMessageAggregator::RoundMessageAggregatorFactory& roundMessageAggregatorFactory)
				: MaxResponseSize(maxResponseSize)
				, MinFinalizationRound(round)
				, MaxFinalizationRound(round)
				, PreviousFinalizedHeightHashPair(previousFinalizedHeightHashPair)
				, RoundMessageAggregatorFactory(roundMessageAggregatorFactory)
		{}

	public:
		uint64_t MaxResponseSize;
		model::FinalizationRound MinFinalizationRound;
		model::FinalizationRound MaxFinalizationRound;
		model::HeightHashPair PreviousFinalizedHeightHashPair;
		MultiRoundMessageAggregator::RoundMessageAggregatorFactory RoundMessageAggregatorFactory;
		std::map<model::FinalizationRound, std::unique_ptr<RoundMessageAggregator>> RoundMessageAggregators;
	};

	// endregion

	// region MultiRoundMessageAggregatorView

	MultiRoundMessageAggregatorView::MultiRoundMessageAggregatorView(
			const MultiRoundMessageAggregatorState& state,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_state(state)
			, m_readLock(std::move(readLock))
	{}

	size_t MultiRoundMessageAggregatorView::size() const {
		return m_state.RoundMessageAggregators.size();
	}

	model::FinalizationRound MultiRoundMessageAggregatorView::minFinalizationRound() const {
		return m_state.MinFinalizationRound;
	}

	model::FinalizationRound MultiRoundMessageAggregatorView::maxFinalizationRound() const {
		return m_state.MaxFinalizationRound;
	}

	const RoundContext* MultiRoundMessageAggregatorView::tryGetRoundContext(const model::FinalizationRound& round) const {
		auto iter = m_state.RoundMessageAggregators.find(round);
		return m_state.RoundMessageAggregators.cend() == iter ? nullptr : &iter->second->roundContext();
	}

	model::HeightHashPair MultiRoundMessageAggregatorView::findEstimate(const model::FinalizationRound& round) const {
		const auto& roundMessageAggregators = m_state.RoundMessageAggregators;
		for (auto iter = roundMessageAggregators.crbegin(); roundMessageAggregators.crend() != iter; ++iter) {
			if (iter->first > round)
				continue;

			auto estimateResultPair = iter->second->roundContext().tryFindEstimate();
			if (estimateResultPair.second)
				return estimateResultPair.first;
		}

		return m_state.PreviousFinalizedHeightHashPair;
	}

	BestPrecommitDescriptor MultiRoundMessageAggregatorView::tryFindBestPrecommit() const {
		const auto& roundMessageAggregators = m_state.RoundMessageAggregators;
		for (auto iter = roundMessageAggregators.crbegin(); roundMessageAggregators.crend() != iter; ++iter) {
			auto bestPrecommitResultPair = iter->second->roundContext().tryFindBestPrecommit();
			if (bestPrecommitResultPair.second) {
				BestPrecommitDescriptor descriptor;
				descriptor.Round = iter->first;
				descriptor.Target = bestPrecommitResultPair.first;
				descriptor.Proof = iter->second->unknownMessages({});
				return descriptor;
			}
		}

		return BestPrecommitDescriptor();
	}

	model::ShortHashRange MultiRoundMessageAggregatorView::shortHashes() const {
		std::vector<utils::ShortHash> shortHashes;
		for (const auto& pair : m_state.RoundMessageAggregators) {
			auto roundShortHashes = pair.second->shortHashes();
			shortHashes.insert(shortHashes.end(), roundShortHashes.cbegin(), roundShortHashes.cend());
		}

		return model::ShortHashRange::CopyFixed(reinterpret_cast<const uint8_t*>(shortHashes.data()), shortHashes.size());
	}

	RoundMessageAggregator::UnknownMessages MultiRoundMessageAggregatorView::unknownMessages(
			const model::FinalizationRoundRange& roundRange,
			const utils::ShortHashesSet& knownShortHashes) const {
		uint64_t totalSize = 0;
		RoundMessageAggregator::UnknownMessages allMessages;
		for (const auto& pair : m_state.RoundMessageAggregators) {
			if (!model::IsInRange(roundRange, pair.first))
				continue;

			for (const auto& pMessage : pair.second->unknownMessages(knownShortHashes)) {
				totalSize += pMessage->Size;
				if (totalSize > m_state.MaxResponseSize)
					return allMessages;

				allMessages.push_back(pMessage);
			}
		}

		return allMessages;
	}

	// endregion

	// region MultiRoundMessageAggregatorModifier

	MultiRoundMessageAggregatorModifier::MultiRoundMessageAggregatorModifier(
			MultiRoundMessageAggregatorState& state,
			utils::SpinReaderWriterLock::WriterLockGuard&& writeLock)
			: m_state(state)
			, m_writeLock(std::move(writeLock))
	{}

	void MultiRoundMessageAggregatorModifier::setMaxFinalizationRound(const model::FinalizationRound& round) {
		if (m_state.MinFinalizationRound > round)
			CATAPULT_THROW_INVALID_ARGUMENT("cannot set max finalization round below min");

		m_state.MaxFinalizationRound = round;

		CATAPULT_LOG(trace) << "set max finalization round to " << m_state.MaxFinalizationRound;
	}

	RoundMessageAggregatorAddResult MultiRoundMessageAggregatorModifier::add(const std::shared_ptr<model::FinalizationMessage>& pMessage) {
		auto messageRound = pMessage->Data().StepIdentifier.Round();
		if (m_state.MinFinalizationRound > messageRound || m_state.MaxFinalizationRound < messageRound) {
			CATAPULT_LOG(warning)
					<< "rejecting message with round " << messageRound
					<< ", min round " << m_state.MinFinalizationRound
					<< ", max round " << m_state.MaxFinalizationRound;
			return RoundMessageAggregatorAddResult::Failure_Invalid_Point;
		}

		auto iter = m_state.RoundMessageAggregators.find(messageRound);
		if (m_state.RoundMessageAggregators.cend() == iter) {
			auto pRoundAggregator = m_state.RoundMessageAggregatorFactory(messageRound);
			iter = m_state.RoundMessageAggregators.emplace(messageRound, std::move(pRoundAggregator)).first;
		}

		return iter->second->add(pMessage);
	}

	void MultiRoundMessageAggregatorModifier::prune(FinalizationEpoch epoch) {
		auto& roundMessageAggregators = m_state.RoundMessageAggregators;

		auto iter = roundMessageAggregators.lower_bound({ epoch, FinalizationPoint(0) });
		roundMessageAggregators.erase(roundMessageAggregators.begin(), iter);

		m_state.MinFinalizationRound = roundMessageAggregators.cend() == iter ? m_state.MaxFinalizationRound : iter->first;

		CATAPULT_LOG(trace) << "set min finalization round to " << m_state.MinFinalizationRound << " after pruning at epoch " << epoch;
	}

	// endregion

	// region MultiRoundMessageAggregator

	MultiRoundMessageAggregator::MultiRoundMessageAggregator(
			uint64_t maxResponseSize,
			const model::FinalizationRound& round,
			const model::HeightHashPair& previousFinalizedHeightHashPair,
			const RoundMessageAggregatorFactory& roundMessageAggregatorFactory)
			: m_pState(std::make_unique<MultiRoundMessageAggregatorState>(
					maxResponseSize,
					round,
					previousFinalizedHeightHashPair,
					roundMessageAggregatorFactory)) {
		CATAPULT_LOG(debug) << "creating multi round message aggregator starting at round " << round;
	}

	MultiRoundMessageAggregator::~MultiRoundMessageAggregator() = default;

	MultiRoundMessageAggregatorView MultiRoundMessageAggregator::view() const {
		auto readLock = m_lock.acquireReader();
		return MultiRoundMessageAggregatorView(*m_pState, std::move(readLock));
	}

	MultiRoundMessageAggregatorModifier MultiRoundMessageAggregator::modifier() {
		auto writeLock = m_lock.acquireWriter();
		return MultiRoundMessageAggregatorModifier(*m_pState, std::move(writeLock));
	}

	// endregion
}}
