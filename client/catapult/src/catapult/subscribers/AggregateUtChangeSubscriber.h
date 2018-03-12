#pragma once
#include "BasicAggregateSubscriber.h"
#include "catapult/cache/UtChangeSubscriber.h"

namespace catapult { namespace subscribers {

	/// Aggregate unconfirmed transactions change subscriber.
	template<typename TUtChangeSubscriber = cache::UtChangeSubscriber>
	class AggregateUtChangeSubscriber : public BasicAggregateSubscriber<TUtChangeSubscriber>, public cache::UtChangeSubscriber {
	public:
		using BasicAggregateSubscriber<TUtChangeSubscriber>::BasicAggregateSubscriber;

	public:
		void notifyAdds(const TransactionInfos& transactionInfos) override {
			this->forEach([&transactionInfos](auto& subscriber) { subscriber.notifyAdds(transactionInfos); });
		}

		void notifyRemoves(const TransactionInfos& transactionInfos) override {
			this->forEach([&transactionInfos](auto& subscriber) { subscriber.notifyRemoves(transactionInfos); });
		}

		void flush() override {
			this->forEach([](auto& subscriber) { subscriber.flush(); });
		}
	};
}}
