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

	FinalizationOrchestrator::FinalizationOrchestrator(
			const VotingStatus& votingStatus,
			const StageAdvancerFactory& stageAdvancerFactory,
			const MessageSink& messageSink,
			std::unique_ptr<FinalizationMessageFactory>&& pMessageFactory)
			: m_pointRaw(votingStatus.Point.unwrap())
			, m_stageAdvancerFactory(stageAdvancerFactory)
			, m_messageSink(messageSink)
			, m_pMessageFactory(std::move(pMessageFactory))
			, m_hasSentPrevote(votingStatus.HasSentPrevote)
			, m_hasSentPrecommit(votingStatus.HasSentPrecommit) {
		CATAPULT_LOG(debug)
				<< "creating finalization orchestrator starting at point " << m_pointRaw
				<< " (has sent prevote? " << m_hasSentPrevote << ")"
				<< " (has sent precommit? " << m_hasSentPrecommit << ")";
	}

	FinalizationPoint FinalizationOrchestrator::point() const {
		return FinalizationPoint(m_pointRaw);
	}

	bool FinalizationOrchestrator::hasSentPrevote() const {
		return m_hasSentPrevote;
	}

	bool FinalizationOrchestrator::hasSentPrecommit() const {
		return m_hasSentPrecommit;
	}

	void FinalizationOrchestrator::poll(Timestamp time) {
		// on first call to poll, don't call startRound in order to use original values for m_hasSentPrevote and m_hasSentPrecommit
		if (!m_pStageAdvancer)
			m_pStageAdvancer = m_stageAdvancerFactory(point(), time);

		if (!m_hasSentPrevote && m_pStageAdvancer->canSendPrevote(time)) {
			m_messageSink(m_pMessageFactory->createPrevote(FinalizationPoint(m_pointRaw)));
			m_hasSentPrevote = true;
		}

		model::HeightHashPair commitTarget;
		if (!m_hasSentPrecommit && m_pStageAdvancer->canSendPrecommit(time, commitTarget)) {
			m_messageSink(m_pMessageFactory->createPrecommit(FinalizationPoint(m_pointRaw), commitTarget.Height, commitTarget.Hash));
			m_hasSentPrecommit = true;
		}

		if (m_hasSentPrecommit && m_pStageAdvancer->canStartNextRound()) {
			++m_pointRaw;
			startRound(time);
		}
	}

	void FinalizationOrchestrator::startRound(Timestamp time) {
		m_hasSentPrevote = false;
		m_hasSentPrecommit = false;
		m_pStageAdvancer = m_stageAdvancerFactory(point(), time);
	}

	action CreateFinalizer(
			MultiRoundMessageAggregator& messageAggregator,
			subscribers::FinalizationSubscriber& subscriber,
			io::ProofStorageCache& proofStorage) {
		return [&messageAggregator, &subscriber, &proofStorage]() {
			auto bestPrecommitDescriptor = messageAggregator.view().tryFindBestPrecommit();
			if (FinalizationPoint(0) == bestPrecommitDescriptor.Point)
				return;

			if (proofStorage.view().statistics().Height == bestPrecommitDescriptor.Target.Height)
				return;

			auto pProof = CreateFinalizationProof(
					{ bestPrecommitDescriptor.Point, bestPrecommitDescriptor.Target.Height, bestPrecommitDescriptor.Target.Hash },
					bestPrecommitDescriptor.Proof);
			proofStorage.modifier().saveProof(*pProof);
			subscriber.notifyFinalizedBlock(
					bestPrecommitDescriptor.Target.Height,
					bestPrecommitDescriptor.Target.Hash,
					bestPrecommitDescriptor.Point);

			// TODO: prune messages here
			// messageAggregator.modifier().prune();
		};
	}
}}
