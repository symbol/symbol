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
			utils::FileSize memorySizeForAccount(const Key& key) const override {
				return modifier().memorySizeForAccount(key);
			}

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
