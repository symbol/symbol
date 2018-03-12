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
