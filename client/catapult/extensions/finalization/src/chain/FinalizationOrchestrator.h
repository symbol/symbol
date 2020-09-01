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

#pragma once
#include "FinalizationMessageFactory.h"
#include "FinalizationStageAdvancer.h"

namespace catapult {
	namespace chain { class MultiRoundMessageAggregator; }
	namespace io { class ProofStorageCache; }
	namespace subscribers { class FinalizationSubscriber; }
}

namespace catapult { namespace chain {

	/// Voting status information.
	struct VotingStatus {
		/// Current finalization point.
		FinalizationPoint Point;

		/// \c true if prevote has been sent for current point.
		bool HasSentPrevote = false;

		/// \c true if precommit has been sent for current point.
		bool HasSentPrecommit = false;
	};

	/// Orchestrates finalization progress.
	class FinalizationOrchestrator {
	private:
		using StageAdvancerFactory = std::function<std::unique_ptr<FinalizationStageAdvancer> (FinalizationPoint, Timestamp)>;
		using MessageSink = consumer<std::unique_ptr<model::FinalizationMessage>&&>;

	public:
		/// Creates an orchestrator around \a votingStatus, \a stageAdvancerFactory, \a messageSink and \a pMessageFactory.
		FinalizationOrchestrator(
				const VotingStatus& votingStatus,
				const StageAdvancerFactory& stageAdvancerFactory,
				const MessageSink& messageSink,
				std::unique_ptr<FinalizationMessageFactory>&& pMessageFactory);

	public:
		/// Gets the current finalization \a point.
		FinalizationPoint point() const;

		/// Returns \c true if a prevote has been sent for the current round.
		bool hasSentPrevote() const;

		/// Returns \c true if a precommit has been sent for the current round.
		bool hasSentPrecommit() const;

	public:
		/// Checks progress given the current \a time.
		void poll(Timestamp time);

	private:
		void startRound(Timestamp time);

	private:
		std::atomic<uint64_t> m_pointRaw;
		StageAdvancerFactory m_stageAdvancerFactory;
		MessageSink m_messageSink;
		std::unique_ptr<FinalizationMessageFactory> m_pMessageFactory;

		std::atomic_bool m_hasSentPrevote;
		std::atomic_bool m_hasSentPrecommit;
		std::unique_ptr<FinalizationStageAdvancer> m_pStageAdvancer;
	};

	/// Creates a finalizer that finalizes as many blocks as possible given \a messageAggregator, \a subscriber and \a proofStorage
	action CreateFinalizer(
			MultiRoundMessageAggregator& messageAggregator,
			subscribers::FinalizationSubscriber& subscriber,
			io::ProofStorageCache& proofStorage);
}}
