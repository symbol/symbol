#pragma once
#include "BasicAggregateSubscriber.h"
#include "NodeSubscriber.h"

namespace catapult { namespace subscribers {

	/// Aggregate node subscriber.
	template<typename TNodeSubscriber = NodeSubscriber>
	class AggregateNodeSubscriber : public BasicAggregateSubscriber<TNodeSubscriber>, public NodeSubscriber {
	public:
		using BasicAggregateSubscriber<TNodeSubscriber>::BasicAggregateSubscriber;

	public:
		void notifyNode(const ionet::Node& node) override {
			this->forEach([&node](auto& subscriber) { subscriber.notifyNode(node); });
		}
	};
}}
