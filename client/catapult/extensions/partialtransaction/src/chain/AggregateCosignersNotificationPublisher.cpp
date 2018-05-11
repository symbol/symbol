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

#include "AggregateCosignersNotificationPublisher.h"
#include "plugins/txes/aggregate/src/model/AggregateNotifications.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/WeakCosignedTransactionInfo.h"

namespace catapult { namespace chain {

	namespace {
		constexpr bool IsAggregate(model::EntityType type) {
			return model::Entity_Type_Aggregate_Complete == type || model::Entity_Type_Aggregate_Bonded == type;
		}

		const auto& CoerceToAggregate(const model::Transaction& transaction) {
			if (!IsAggregate(transaction.Type))
				CATAPULT_THROW_INVALID_ARGUMENT("AggregateNotificationPublisher only supports aggregate transactions");

			const auto& aggregate = static_cast<const model::AggregateTransaction&>(transaction);
			if (0 != aggregate.CosignaturesCount())
				CATAPULT_THROW_INVALID_ARGUMENT("AggregateNotificationPublisher only supports aggregates without cosignatures");

			return aggregate;
		}
	}

	void AggregateCosignersNotificationPublisher::publish(
			const model::WeakCosignedTransactionInfo& transactionInfo,
			model::NotificationSubscriber& sub) const {
		// publish aggregate notifications
		const auto& aggregate = CoerceToAggregate(transactionInfo.transaction());
		auto numCosignatures = transactionInfo.cosignatures().size();
		sub.notify(model::AggregateCosignaturesNotification(
				aggregate.Signer,
				static_cast<uint32_t>(std::distance(aggregate.Transactions().cbegin(), aggregate.Transactions().cend())),
				aggregate.TransactionsPtr(),
				numCosignatures,
				transactionInfo.cosignatures().data()));

		// publish all sub-transaction information
		for (const auto& subTransaction : aggregate.Transactions()) {
			// - generic sub-transaction notification
			sub.notify(model::AggregateEmbeddedTransactionNotification(
					aggregate.Signer,
					subTransaction,
					numCosignatures,
					transactionInfo.cosignatures().data()));
		}
	}
}}
