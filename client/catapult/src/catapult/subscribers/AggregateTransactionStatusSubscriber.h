#pragma once
#include "BasicAggregateSubscriber.h"
#include "TransactionStatusSubscriber.h"

namespace catapult { namespace subscribers {

	/// Aggregate transaction status subscriber.
	template<typename TTransactionStatusSubscriber = TransactionStatusSubscriber>
	class AggregateTransactionStatusSubscriber
			: public BasicAggregateSubscriber<TTransactionStatusSubscriber>
			, public TransactionStatusSubscriber {
	public:
		using BasicAggregateSubscriber<TTransactionStatusSubscriber>::BasicAggregateSubscriber;

	public:
		void notifyStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status) override {
			this->forEach([&transaction, &hash, status](auto& subscriber) { subscriber.notifyStatus(transaction, hash, status); });
		}

		void flush() override {
			this->forEach([](auto& subscriber) { subscriber.flush(); });
		}
	};
}}
