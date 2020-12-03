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

#include "FinalizationStageAdvancer.h"
#include "MultiRoundMessageAggregator.h"
#include "RoundContext.h"

namespace catapult { namespace chain {

	namespace {
		class PollingTimer {
		public:
			PollingTimer(Timestamp startTime, const utils::TimeSpan& stepDuration)
					: m_startTime(startTime)
					, m_stepDuration(stepDuration)
			{}

		public:
			bool isElapsed(Timestamp time, uint16_t numSteps) const {
				return time >= m_startTime + Timestamp(numSteps * m_stepDuration.millis());
			}

		private:
			Timestamp m_startTime;
			utils::TimeSpan m_stepDuration;
		};

		class DefaultFinalizationStageAdvancer : public FinalizationStageAdvancer {
		public:
			DefaultFinalizationStageAdvancer(
					const model::FinalizationRound& round,
					Timestamp time,
					const utils::TimeSpan& stepDuration,
					const MultiRoundMessageAggregator& messageAggregator)
					: m_round(round)
					, m_timer(time, stepDuration)
					, m_messageAggregator(messageAggregator) {
				CATAPULT_LOG(debug) << "creating finalization stage advancer for round " << round << " at " << time;
			}

		public:
			bool canSendPrevote(Timestamp time) const override {
				if (m_timer.isElapsed(time, 1)) {
					CATAPULT_LOG(trace) << "can send prevote - time elapsed";
					return true;
				}

				return requireRoundContext([](const auto&, const auto& roundContext) {
					if (roundContext.isCompletable()) {
						CATAPULT_LOG(trace) << "can send prevote - completable";
						return true;
					}

					CATAPULT_LOG(trace) << "cannot send prevote - not completable";
					return false;
				});
			}

			bool canSendPrecommit(Timestamp time, model::HeightHashPair& target) const override {
				return requireRoundContext([this, time, &target](const auto& messageAggregatorView, const auto& roundContext) {
					auto bestPrevoteResultPair = roundContext.tryFindBestPrevote();
					if (!bestPrevoteResultPair.second) {
						CATAPULT_LOG(trace) << "cannot send precommit - no best prevote";
						return false;
					}

					auto estimate = messageAggregatorView.findEstimate(m_round - FinalizationPoint(1));
					if (!roundContext.isDescendant(estimate, bestPrevoteResultPair.first)) {
						CATAPULT_LOG(trace) << "cannot send precommit - not descendant";
						return false;
					}

					if (m_timer.isElapsed(time, 2)) {
						CATAPULT_LOG(trace) << "can send precommit - time elapsed";
					} else if (roundContext.isCompletable()) {
						CATAPULT_LOG(trace) << "can send precommit - completable";
					} else {
						CATAPULT_LOG(trace) << "cannot send precommit - neither time elapsed nor completable";
						return false;
					}

					target = bestPrevoteResultPair.first;
					return true;
				});
			}

			bool canStartNextRound() const override {
				return requireRoundContext([](const auto&, const auto& roundContext) {
					if (roundContext.isCompletable()) {
						CATAPULT_LOG(trace) << "can start next round - completable";
						return true;
					}

					CATAPULT_LOG(trace) << "cannot start next round - not completable";
					return false;
				});
			}

		private:
			bool requireRoundContext(const predicate<const MultiRoundMessageAggregatorView&, const RoundContext&>& predicate) const {
				auto messageAggregatorView = m_messageAggregator.view();

				const auto* pCurrentRoundContext = messageAggregatorView.tryGetRoundContext(m_round);
				if (!pCurrentRoundContext)
					return false;

				return predicate(messageAggregatorView, *pCurrentRoundContext);
			}

		private:
			model::FinalizationRound m_round;
			PollingTimer m_timer;
			const MultiRoundMessageAggregator& m_messageAggregator;
		};
	}

	std::unique_ptr<FinalizationStageAdvancer> CreateFinalizationStageAdvancer(
			const model::FinalizationRound& round,
			Timestamp time,
			const utils::TimeSpan& stepDuration,
			const MultiRoundMessageAggregator& messageAggregator) {
		return std::make_unique<DefaultFinalizationStageAdvancer>(round, time, stepDuration, messageAggregator);
	}
}}
