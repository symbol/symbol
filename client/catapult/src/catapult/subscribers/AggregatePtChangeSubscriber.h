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

#pragma once
#include "BasicAggregateSubscriber.h"
#include "catapult/cache_tx/PtChangeSubscriber.h"

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

		void notifyAddCosignature(const model::TransactionInfo& parentTransactionInfo, const model::Cosignature& cosignature) override {
			this->forEach([&parentTransactionInfo, &cosignature](auto& subscriber) {
				subscriber.notifyAddCosignature(parentTransactionInfo, cosignature);
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
