#pragma once
#include "BasicAggregateSubscriber.h"
#include "catapult/cache/PtChangeSubscriber.h"

namespace catapult { namespace subscribers {

	/// Aggregate partial transactions change subscriber.
	template<typename TPtChangeSubscriber = cache::PtChangeSubscriber>
	class AggregatePtChangeSubscriber : public BasicAggregateSubscriber<TPtChangeSubscriber>, public cache::PtChangeSubscriber {
	public:
		using BasicAggregateSubscriber<TPtChangeSubscriber>::BasicAggregateSubscriber;

	public:
		void notifyAddPartials(const TransactionInfos& transactionInfos) override {
			this->forEach([&transactionInfos](auto& subscriber) { subscriber.notifyAddPartials(transactionInfos); });
		}

		void notifyAddCosignature(
				const model::TransactionInfo& parentTransactionInfo,
				const Key& signer,
				const Signature& signature) override {
			this->forEach([&parentTransactionInfo, &signer, &signature](auto& subscriber) {
				subscriber.notifyAddCosignature(parentTransactionInfo, signer, signature);
			});
		}

		void notifyRemovePartials(const TransactionInfos& transactionInfos) override {
			this->forEach([&transactionInfos](auto& subscriber) { subscriber.notifyRemovePartials(transactionInfos); });
		}

		void flush() override {
			this->forEach([](auto& subscriber) { subscriber.flush(); });
		}
	};
}}
