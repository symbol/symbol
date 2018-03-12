#pragma once
#include "BasicAggregateSubscriber.h"
#include "catapult/io/BlockChangeSubscriber.h"

namespace catapult { namespace subscribers {

	/// Aggregate block change subscriber.
	template<typename TBlockChangeSubscriber = io::BlockChangeSubscriber>
	class AggregateBlockChangeSubscriber : public BasicAggregateSubscriber<TBlockChangeSubscriber>, public io::BlockChangeSubscriber {
	public:
		using BasicAggregateSubscriber<TBlockChangeSubscriber>::BasicAggregateSubscriber;

	public:
		void notifyBlock(const model::BlockElement& blockElement) override {
			this->forEach([&blockElement](auto& subscriber) { subscriber.notifyBlock(blockElement); });
		}

		void notifyDropBlocksAfter(Height height) override {
			this->forEach([height](auto& subscriber) { subscriber.notifyDropBlocksAfter(height); });
		}
	};
}}
