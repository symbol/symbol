#pragma once
#include "catapult/subscribers/StateChangeSubscriber.h"

namespace catapult { namespace mocks {

	/// Mock noop state change subscriber implementation.
	class MockStateChangeSubscriber : public subscribers::StateChangeSubscriber {
	public:
		/// Creates a subscriber.
		MockStateChangeSubscriber()
				: m_numScoreChanges(0)
				, m_numStateChanges(0)
		{}

	public:
		/// Gets the number of score changes.
		size_t numScoreChanges() const {
			return m_numScoreChanges;
		}

		/// Gets the number of state changes.
		size_t numStateChanges() const {
			return m_numStateChanges;
		}

		/// Gets the last chain score.
		model::ChainScore lastChainScore() const {
			return m_lastChainScore;
		}

	public:
		void notifyScoreChange(const model::ChainScore& score) override {
			m_lastChainScore = score;
			++m_numScoreChanges;
		}

		void notifyStateChange(const consumers::StateChangeInfo&) override {
			++m_numStateChanges;
		}

	private:
		size_t m_numScoreChanges;
		size_t m_numStateChanges;
		model::ChainScore m_lastChainScore;
	};
}}
