#pragma once
#include "HashCheckOptions.h"
#include "InputUtils.h"
#include "catapult/chain/ChainFunctions.h"
#include "catapult/disruptor/DisruptorConsumer.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/validators/ParallelValidationPolicy.h"

namespace catapult { namespace model { class NotificationPublisher; } }

namespace catapult { namespace consumers {

	/// Creates a consumer that calculates hashes of all entities using \a transactionRegistry.
	disruptor::TransactionConsumer CreateTransactionHashCalculatorConsumer(const model::TransactionRegistry& transactionRegistry);

	/// Creates a consumer that checks entities for previous processing based on their hash.
	/// \a timeSupplier is used for generating timestamps and \a options specifies additional cache options.
	/// \a knownHashPredicate returns \c true for known hashes.
	disruptor::TransactionConsumer CreateTransactionHashCheckConsumer(
			const chain::TimeSupplier& timeSupplier,
			const HashCheckOptions& options,
			const chain::KnownHashPredicate& knownHashPredicate);

	/// Creates a consumer that extracts all addresses affected by transactions using \a notificationPublisher.
	disruptor::TransactionConsumer CreateTransactionAddressExtractionConsumer(const model::NotificationPublisher& notificationPublisher);

	/// Creates a consumer that runs stateless validation using \a pValidator and the specified policy
	/// (\a pValidationPolicy) and calls \a failedTransactionSink for each failure.
	disruptor::TransactionConsumer CreateTransactionStatelessValidationConsumer(
			const std::shared_ptr<const validators::stateless::AggregateEntityValidator>& pValidator,
			const std::shared_ptr<const validators::ParallelValidationPolicy>& pValidationPolicy,
			const chain::FailedTransactionSink& failedTransactionSink);

	/// Prototype for a function that is called with new transactions.
	using NewTransactionsSink = consumer<TransactionInfos&&>;

	/// Creates a consumer that calls \a newTransactionsSink with all new transactions.
	/// \note This consumer must be last because it destroys the input.
	disruptor::DisruptorConsumer CreateNewTransactionsConsumer(const NewTransactionsSink& newTransactionsSink);
}}
