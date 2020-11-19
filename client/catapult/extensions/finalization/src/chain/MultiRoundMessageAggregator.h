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

#pragma once
#include "RoundMessageAggregator.h"
#include "catapult/model/FinalizationRound.h"
#include "catapult/model/HeightHashPair.h"
#include "catapult/utils/SpinReaderWriterLock.h"

namespace catapult {
	namespace chain { struct MultiRoundMessageAggregatorState; }
	namespace model { struct FinalizationRoundRange; }
}

namespace catapult { namespace chain {

	// region BestPrecommitDescriptor

	/// Describes the best precommit.
	struct BestPrecommitDescriptor {
		/// Finalization round.
		model::FinalizationRound Round;

		/// Height hash pair corresponding to the block that can be finalized.
		model::HeightHashPair Target;

		/// Proof of the finalization.
		std::vector<std::shared_ptr<const model::FinalizationMessage>> Proof;
	};

	// endregion

	// region MultiRoundMessageAggregatorView

	/// Read only view on top of multi round message aggregator.
	class MultiRoundMessageAggregatorView : utils::MoveOnly {
	public:
		/// Creates a view around \a state with lock context \a readLock.
		MultiRoundMessageAggregatorView(
				const MultiRoundMessageAggregatorState& state,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Gets the number of round message aggregators.
		size_t size() const;

		/// Gets the minimum finalization round of messages that can be accepted.
		model::FinalizationRound minFinalizationRound() const;

		/// Gets the maximum finalization round of messages that can be accepted.
		model::FinalizationRound maxFinalizationRound() const;

		/// Tries to get the round context for the specified \a round.
		const RoundContext* tryGetRoundContext(const model::FinalizationRound& round) const;

		/// Finds the estimate for the specified \a round.
		model::HeightHashPair findEstimate(const model::FinalizationRound& round) const;

		/// Finds the candidate with the largest height that has at least threshold prevotes and precommits, if any.
		BestPrecommitDescriptor tryFindBestPrecommit() const;

		/// Gets a range of short hashes of all messages in the cache.
		/// \note Each short hash consists of the first 4 bytes of the complete hash.
		model::ShortHashRange shortHashes() const;

		/// Gets all finalization messages with a finalization round in \a roundRange that do not have a
		/// short hash in \a knownShortHashes.
		RoundMessageAggregator::UnknownMessages unknownMessages(
				const model::FinalizationRoundRange& roundRange,
				const utils::ShortHashesSet& knownShortHashes) const;

	private:
		const MultiRoundMessageAggregatorState& m_state;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	// endregion

	// region MultiRoundMessageAggregatorModifier

	/// Write only view on top of multi round message aggregator.
	class MultiRoundMessageAggregatorModifier : utils::MoveOnly {
	public:
		/// Creates a view around \a state with lock context \a writeLock.
		MultiRoundMessageAggregatorModifier(
				MultiRoundMessageAggregatorState& state,
				utils::SpinReaderWriterLock::WriterLockGuard&& writeLock);

	public:
		/// Sets the maximum finalization \a round of messages that can be accepted.
		void setMaxFinalizationRound(const model::FinalizationRound& round);

		/// Adds a finalization message (\a pMessage) to the aggregator.
		/// \note Message is a shared_ptr because it is detached from an EntityRange and is kept alive with its associated step.
		RoundMessageAggregatorAddResult add(const std::shared_ptr<model::FinalizationMessage>& pMessage);

		/// Prunes this aggregator by removing all rounds with an epoch less than \a epoch.
		void prune(FinalizationEpoch epoch);

	private:
		MultiRoundMessageAggregatorState& m_state;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
	};

	// endregion

	// region MultiRoundMessageAggregator

	/// Aggregates finalization messages across multiple finalization points.
	class MultiRoundMessageAggregator {
	public:
		using RoundMessageAggregatorFactory = std::function<std::unique_ptr<RoundMessageAggregator> (const model::FinalizationRound&)>;

	public:
		/// Creates an aggregator around \a maxResponseSize, the current finalization \a round,
		/// the previous finalized height hash pair (\a previousFinalizedHeightHashPair) and a factory for creating
		/// round message aggregators (\a roundMessageAggregatorFactory).
		MultiRoundMessageAggregator(
				uint64_t maxResponseSize,
				const model::FinalizationRound& round,
				const model::HeightHashPair& previousFinalizedHeightHashPair,
				const RoundMessageAggregatorFactory& roundMessageAggregatorFactory);

		/// Destroys the aggregator.
		~MultiRoundMessageAggregator();

	public:
		/// Gets a read only view of the aggregator.
		MultiRoundMessageAggregatorView view() const;

		/// Gets a write only view of the aggregator.
		MultiRoundMessageAggregatorModifier modifier();

	private:
		std::unique_ptr<MultiRoundMessageAggregatorState> m_pState;
		mutable utils::SpinReaderWriterLock m_lock;
	};

	// endregion
}}
