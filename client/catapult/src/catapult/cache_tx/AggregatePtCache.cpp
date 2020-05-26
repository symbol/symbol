/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "AggregatePtCache.h"
#include "BasicAggregateTransactionsCache.h"
#include "PtChangeSubscriber.h"

namespace catapult { namespace cache {

	namespace {
		struct PtTraits {
			using CacheType = PtCache;
			using ChangeSubscriberType = PtChangeSubscriber;
			using CacheModifierType = PtCacheModifier;
			using CacheModifierProxyType = PtCacheModifierProxy;
		};

		struct PtChangeSubscriberTraits {
			using TransactionInfoType = model::DetachedTransactionInfo;

			static void NotifyAdds(PtChangeSubscriber& subscriber, const PtChangeSubscriber::TransactionInfos& transactionInfos) {
				subscriber.notifyAddPartials(transactionInfos);
			}

			static void NotifyRemoves(PtChangeSubscriber& subscriber, const PtChangeSubscriber::TransactionInfos& transactionInfos) {
				subscriber.notifyRemovePartials(transactionInfos);
			}

			static model::TransactionInfo ToTransactionInfo(const TransactionInfoType& transactionInfo) {
				// the PtCache does not support merkle component hashes, so pass in a zeroed out merkle component hash
				model::TransactionInfo transactionInfoWithMerkleHash(transactionInfo.pEntity, transactionInfo.EntityHash);
				transactionInfoWithMerkleHash.OptionalExtractedAddresses = transactionInfo.OptionalExtractedAddresses;
				transactionInfoWithMerkleHash.MerkleComponentHash = Hash256();
				return transactionInfoWithMerkleHash;
			}
		};

		class AggregatePtCacheModifier : public BasicAggregateTransactionsCacheModifier<PtTraits, PtChangeSubscriberTraits> {
		public:
			using BaseType = BasicAggregateTransactionsCacheModifier<PtTraits, PtChangeSubscriberTraits>;
			using BaseType::BasicAggregateTransactionsCacheModifier;
			using BaseType::add;

		public:
			model::DetachedTransactionInfo add(const Hash256& parentHash, const model::Cosignature& cosignature) override {
				auto parentInfo = modifier().add(parentHash, cosignature);
				if (parentInfo)
					subscriber().notifyAddCosignature(PtChangeSubscriberTraits::ToTransactionInfo(parentInfo), cosignature);

				return parentInfo;
			}

			std::vector<model::DetachedTransactionInfo> prune(Timestamp timestamp) override {
				return removeAll(modifier().prune(timestamp));
			}

			std::vector<model::DetachedTransactionInfo> prune(const predicate<const Hash256&>& hashPredicate) override {
				return removeAll(modifier().prune(hashPredicate));
			}

		private:
			std::vector<model::DetachedTransactionInfo> removeAll(std::vector<model::DetachedTransactionInfo>&& transactionInfos) {
				for (const auto& transactionInfo : transactionInfos)
					remove(transactionInfo);

				return std::move(transactionInfos);
			}
		};

		using AggregatePtCache = BasicAggregateTransactionsCache<PtTraits, AggregatePtCacheModifier>;
	}

	std::unique_ptr<PtCache> CreateAggregatePtCache(PtCache& ptCache, std::unique_ptr<PtChangeSubscriber>&& pPtChangeSubscriber) {
		return std::make_unique<AggregatePtCache>(ptCache, std::move(pPtChangeSubscriber));
	}
}}
