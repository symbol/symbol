#pragma once
#include "BasicAggregateSubscriber.h"
#include "StateChangeSubscriber.h"

namespace catapult { namespace subscribers {

	/// Aggregate state change subscriber.
	template<typename TStateChangeSubscriber = StateChangeSubscriber>
	class AggregateStateChangeSubscriber : public BasicAggregateSubscriber<TStateChangeSubscriber>, public StateChangeSubscriber {
	public:
		using BasicAggregateSubscriber<TStateChangeSubscriber>::BasicAggregateSubscriber;

	public:
		void notifyScoreChange(const model::ChainScore& chainScore) override {
			this->forEach([&chainScore](auto& subscriber) { subscriber.notifyScoreChange(chainScore); });
		}

		void notifyStateChange(const consumers::StateChangeInfo& stateChangeInfo) override {
			this->forEach([&stateChangeInfo](auto& subscriber) { subscriber.notifyStateChange(stateChangeInfo); });
		}
	};
}}
