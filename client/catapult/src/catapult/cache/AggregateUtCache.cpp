#include "AggregateUtCache.h"
#include "BasicAggregateTransactionsCache.h"
#include "UtChangeSubscriber.h"

namespace catapult { namespace cache {

	namespace {
		struct UtTraits {
			using CacheType = UtCache;
			using ChangeSubscriberType = UtChangeSubscriber;
			using CacheModifierType = UtCacheModifier;
			using CacheModifierProxyType = UtCacheModifierProxy;
		};

		struct UtChangeSubscriberTraits {
			using TransactionInfoType = model::TransactionInfo;

			static void NotifyAdds(UtChangeSubscriber& subscriber, const UtChangeSubscriber::TransactionInfos& transactionInfos) {
				subscriber.notifyAdds(transactionInfos);
			}

			static void NotifyRemoves(UtChangeSubscriber& subscriber, const UtChangeSubscriber::TransactionInfos& transactionInfos) {
				subscriber.notifyRemoves(transactionInfos);
			}

			static const model::TransactionInfo& ToTransactionInfo(const TransactionInfoType& transactionInfo) {
				return transactionInfo;
			}
		};

		class AggregateUtCacheModifier : public BasicAggregateTransactionsCacheModifier<UtTraits, UtChangeSubscriberTraits> {
		public:
			using BasicAggregateTransactionsCacheModifier<UtTraits, UtChangeSubscriberTraits>::BasicAggregateTransactionsCacheModifier;

		public:
			std::vector<model::TransactionInfo> removeAll() override {
				auto transactionInfos = modifier().removeAll();
				for (const auto& transactionInfo : transactionInfos)
					remove(transactionInfo);

				return transactionInfos;
			}
		};

		using AggregateUtCache = BasicAggregateTransactionsCache<UtTraits, AggregateUtCacheModifier>;
	}

	std::unique_ptr<UtCache> CreateAggregateUtCache(UtCache& utCache, std::unique_ptr<UtChangeSubscriber>&& pUtChangeSubscriber) {
		return std::make_unique<AggregateUtCache>(utCache, std::move(pUtChangeSubscriber));
	}
}}
