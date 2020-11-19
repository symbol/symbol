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
#include "FinalizationMessageFactory.h"
#include "FinalizationStageAdvancer.h"
#include "catapult/model/FinalizationRound.h"

namespace catapult {
	namespace chain { class MultiRoundMessageAggregator; }
	namespace io { class ProofStorageCache; }
	namespace subscribers { class FinalizationSubscriber; }
}

namespace catapult { namespace chain {

	/// Voting status information.
	struct VotingStatus {
		/// Current finalization round.
		model::FinalizationRound Round;

		/// \c true if prevote has been sent for current round.
		bool HasSentPrevote = false;

		/// \c true if precommit has been sent for current round.
		bool HasSentPrecommit = false;
	};

	/// Orchestrates finalization progress.
	class FinalizationOrchestrator {
	private:
		using FinalizationStageAdvancerPointer = std::unique_ptr<FinalizationStageAdvancer>;
		using StageAdvancerFactory = std::function<FinalizationStageAdvancerPointer (const model::FinalizationRound&, Timestamp)>;
		using MessagePredicate = predicate<const model::FinalizationMessage&>;
		using MessageSink = consumer<std::unique_ptr<model::FinalizationMessage>&&>;

	public:
		/// Creates an orchestrator around \a votingStatus, \a stageAdvancerFactory, \a messagePredicate, \a messageSink
		/// and \a pMessageFactory.
		FinalizationOrchestrator(
				const VotingStatus& votingStatus,
				const StageAdvancerFactory& stageAdvancerFactory,
				const MessagePredicate& messagePredicate,
				const MessageSink& messageSink,
				std::unique_ptr<FinalizationMessageFactory>&& pMessageFactory);

	public:
		/// Gets the current voting status.
		VotingStatus votingStatus() const;

	public:
		/// Sets the \a epoch.
		void setEpoch(FinalizationEpoch epoch);

		/// Checks progress given the current \a time.
		void poll(Timestamp time);

	private:
		void startRound(Timestamp time);
		void process(std::unique_ptr<model::FinalizationMessage>&& pMessage, const char* description);

	private:
		VotingStatus m_votingStatus;
		StageAdvancerFactory m_stageAdvancerFactory;
		MessagePredicate m_messagePredicate;
		MessageSink m_messageSink;
		std::unique_ptr<FinalizationMessageFactory> m_pMessageFactory;

		std::unique_ptr<FinalizationStageAdvancer> m_pStageAdvancer;
	};

	/// Creates a finalizer that finalizes as many blocks as possible given \a messageAggregator and \a proofStorage.
	action CreateFinalizer(MultiRoundMessageAggregator& messageAggregator, io::ProofStorageCache& proofStorage);
}}
