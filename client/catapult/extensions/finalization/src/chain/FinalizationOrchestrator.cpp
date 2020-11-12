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

#include "FinalizationOrchestrator.h"
#include "MultiRoundMessageAggregator.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "finalization/src/model/FinalizationProofUtils.h"
#include "catapult/subscribers/FinalizationSubscriber.h"

namespace catapult { namespace chain {

	namespace {
		void ClearFlags(VotingStatus& votingStatus) {
			votingStatus.HasSentPrevote = false;
			votingStatus.HasSentPrecommit = false;
		}
	}

	FinalizationOrchestrator::FinalizationOrchestrator(
			const VotingStatus& votingStatus,
			const StageAdvancerFactory& stageAdvancerFactory,
			const MessagePredicate& messagePredicate,
			const MessageSink& messageSink,
			std::unique_ptr<FinalizationMessageFactory>&& pMessageFactory)
			: m_votingStatus(votingStatus)
			, m_stageAdvancerFactory(stageAdvancerFactory)
			, m_messagePredicate(messagePredicate)
			, m_messageSink(messageSink)
			, m_pMessageFactory(std::move(pMessageFactory)) {
		CATAPULT_LOG(debug)
				<< "creating finalization orchestrator starting at round " << m_votingStatus.Round
				<< " (has sent prevote? " << m_votingStatus.HasSentPrevote << ")"
				<< " (has sent precommit? " << m_votingStatus.HasSentPrecommit << ")";
	}

	VotingStatus FinalizationOrchestrator::votingStatus() const {
		return m_votingStatus;
	}

	void FinalizationOrchestrator::setEpoch(FinalizationEpoch epoch) {
		if (epoch < m_votingStatus.Round.Epoch)
			CATAPULT_THROW_INVALID_ARGUMENT("cannot decrease epoch");

		if (epoch == m_votingStatus.Round.Epoch)
			return;

		m_votingStatus.Round = { epoch, FinalizationPoint(1) };
		ClearFlags(m_votingStatus);
		m_pStageAdvancer.reset();
	}

	void FinalizationOrchestrator::poll(Timestamp time) {
		// on first call to poll, don't call startRound in order to use original values for m_votingStatus
		if (!m_pStageAdvancer)
			m_pStageAdvancer = m_stageAdvancerFactory(m_votingStatus.Round, time);

		if (!m_votingStatus.HasSentPrevote && m_pStageAdvancer->canSendPrevote(time)) {
			process(m_pMessageFactory->createPrevote(m_votingStatus.Round), "prevote");
			m_votingStatus.HasSentPrevote = true;
		}

		model::HeightHashPair commitTarget;
		if (!m_votingStatus.HasSentPrecommit && m_pStageAdvancer->canSendPrecommit(time, commitTarget)) {
			process(m_pMessageFactory->createPrecommit(m_votingStatus.Round, commitTarget.Height, commitTarget.Hash), "precommit");
			m_votingStatus.HasSentPrecommit = true;
		}

		if (m_votingStatus.HasSentPrecommit && m_pStageAdvancer->canStartNextRound()) {
			m_votingStatus.Round.Point = m_votingStatus.Round.Point + FinalizationPoint(1);
			startRound(time);
		}
	}

	void FinalizationOrchestrator::startRound(Timestamp time) {
		ClearFlags(m_votingStatus);
		m_pStageAdvancer = m_stageAdvancerFactory(m_votingStatus.Round, time);
	}

	void FinalizationOrchestrator::process(std::unique_ptr<model::FinalizationMessage>&& pMessage, const char* description) {
		if (!pMessage || !m_messagePredicate(*pMessage)) {
			CATAPULT_LOG(debug)
					<< "cannot create " << description << " for " << m_votingStatus.Round << " ("
					<< (pMessage ? "ineligible" : "factory failed") << ")";
			return;
		}

		m_messageSink(std::move(pMessage));
	}

	namespace {
		model::FinalizationStatistics ToFinalizationStatistics(const BestPrecommitDescriptor& bestPrecommitDescriptor) {
			return { bestPrecommitDescriptor.Round, bestPrecommitDescriptor.Target.Height, bestPrecommitDescriptor.Target.Hash };
		}
	}

	action CreateFinalizer(MultiRoundMessageAggregator& messageAggregator, io::ProofStorageCache& proofStorage) {
		return [&messageAggregator, &proofStorage]() {
			auto bestPrecommitDescriptor = messageAggregator.view().tryFindBestPrecommit();
			auto bestPrecommitRound = bestPrecommitDescriptor.Round;
			if (model::FinalizationRound() == bestPrecommitRound)
				return;

			if (proofStorage.view().statistics().Height == bestPrecommitDescriptor.Target.Height)
				return;

			auto statistics = ToFinalizationStatistics(bestPrecommitDescriptor);
			CATAPULT_LOG(info)
					<< "finalization round " << statistics.Round
					<< " reached consensus among "<< bestPrecommitDescriptor.Proof.size() << " messages"
					<< " for block " << statistics.Hash << " at " << statistics.Height;

			auto pProof = CreateFinalizationProof(statistics, bestPrecommitDescriptor.Proof);
			proofStorage.modifier().saveProof(*pProof);

			// prune previous epoch when later epoch has finalized at least one (new) block
			messageAggregator.modifier().prune(bestPrecommitRound.Epoch - FinalizationEpoch(1));
		};
	}
}}
