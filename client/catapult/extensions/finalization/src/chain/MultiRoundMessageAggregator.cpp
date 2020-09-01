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

#include "MultiRoundMessageAggregator.h"
#include "RoundContext.h"
#include "finalization/src/model/FinalizationMessage.h"

namespace catapult { namespace chain {

	// region MultiRoundMessageAggregatorState

	struct MultiRoundMessageAggregatorState {
	public:
		MultiRoundMessageAggregatorState(
				uint64_t maxResponseSize,
				FinalizationPoint finalizationPoint,
				const model::HeightHashPair& previousFinalizedHeightHashPair,
				const MultiRoundMessageAggregator::RoundMessageAggregatorFactory& roundMessageAggregatorFactory)
				: MaxResponseSize(maxResponseSize)
				, MinFinalizationPoint(finalizationPoint)
				, MaxFinalizationPoint(finalizationPoint)
				, PreviousFinalizedHeightHashPair(previousFinalizedHeightHashPair)
				, RoundMessageAggregatorFactory(roundMessageAggregatorFactory)
		{}

	public:
		uint64_t MaxResponseSize;
		FinalizationPoint MinFinalizationPoint;
		FinalizationPoint MaxFinalizationPoint;
		model::HeightHashPair PreviousFinalizedHeightHashPair;
		MultiRoundMessageAggregator::RoundMessageAggregatorFactory RoundMessageAggregatorFactory;
		std::map<FinalizationPoint, std::unique_ptr<RoundMessageAggregator>> RoundMessageAggregators;
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

	FinalizationPoint MultiRoundMessageAggregatorView::minFinalizationPoint() const {
		return m_state.MinFinalizationPoint;
	}

	FinalizationPoint MultiRoundMessageAggregatorView::maxFinalizationPoint() const {
		return m_state.MaxFinalizationPoint;
	}

	const RoundContext* MultiRoundMessageAggregatorView::tryGetRoundContext(FinalizationPoint point) const {
		auto iter = m_state.RoundMessageAggregators.find(point);
		return m_state.RoundMessageAggregators.cend() == iter ? nullptr : &iter->second->roundContext();
	}

	model::HeightHashPair MultiRoundMessageAggregatorView::findEstimate(FinalizationPoint point) const {
		const auto& roundMessageAggregators = m_state.RoundMessageAggregators;
		for (auto iter = roundMessageAggregators.crbegin(); roundMessageAggregators.crend() != iter; ++iter) {
			if (iter->first > point)
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
				descriptor.Point = iter->first;
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
			FinalizationPoint point,
			const utils::ShortHashesSet& knownShortHashes) const {
		uint64_t totalSize = 0;
		RoundMessageAggregator::UnknownMessages allMessages;
		for (const auto& pair : m_state.RoundMessageAggregators) {
			if (pair.first < point)
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

	void MultiRoundMessageAggregatorModifier::setMaxFinalizationPoint(FinalizationPoint point) {
		if (m_state.MinFinalizationPoint > point)
			CATAPULT_THROW_INVALID_ARGUMENT("cannot set max finalization point below min");

		m_state.MaxFinalizationPoint = point;
	}

	RoundMessageAggregatorAddResult MultiRoundMessageAggregatorModifier::add(const std::shared_ptr<model::FinalizationMessage>& pMessage) {
		auto messagePoint = pMessage->StepIdentifier.Point;
		if (m_state.MinFinalizationPoint > messagePoint || m_state.MaxFinalizationPoint < messagePoint)
			return RoundMessageAggregatorAddResult::Failure_Invalid_Point;

		auto iter = m_state.RoundMessageAggregators.find(messagePoint);
		if (m_state.RoundMessageAggregators.cend() == iter) {
			iter = m_state.RoundMessageAggregators.emplace(
					messagePoint,
					m_state.RoundMessageAggregatorFactory(messagePoint, pMessage->Height)).first;
		}

		return iter->second->add(pMessage);
	}

	void MultiRoundMessageAggregatorModifier::prune() {
		auto& roundMessageAggregators = m_state.RoundMessageAggregators;

		auto lastMatchingIter = roundMessageAggregators.cend();
		for (auto iter = roundMessageAggregators.cbegin(); roundMessageAggregators.cend() != iter; ++iter) {
			if (iter->second->roundContext().tryFindBestPrecommit().second)
				lastMatchingIter = iter;
		}

		if (roundMessageAggregators.cend() == lastMatchingIter)
			return;

		auto prevLastMatchingIter = lastMatchingIter;
		while (roundMessageAggregators.cbegin() != prevLastMatchingIter) {
			--prevLastMatchingIter;

			auto estimateResultPair = prevLastMatchingIter->second->roundContext().tryFindEstimate();
			if (estimateResultPair.second) {
				m_state.PreviousFinalizedHeightHashPair = estimateResultPair.first;
				break;
			}
		}

		roundMessageAggregators.erase(roundMessageAggregators.cbegin(), lastMatchingIter);
		m_state.MinFinalizationPoint = lastMatchingIter->first;
	}

	// endregion

	// region MultiRoundMessageAggregator

	MultiRoundMessageAggregator::MultiRoundMessageAggregator(
			uint64_t maxResponseSize,
			FinalizationPoint finalizationPoint,
			const model::HeightHashPair& previousFinalizedHeightHashPair,
			const RoundMessageAggregatorFactory& roundMessageAggregatorFactory)
			: m_pState(std::make_unique<MultiRoundMessageAggregatorState>(
					maxResponseSize,
					finalizationPoint,
					previousFinalizedHeightHashPair,
					roundMessageAggregatorFactory)) {
		CATAPULT_LOG(debug) << "creating multi round message aggregator starting at point " << finalizationPoint;
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
