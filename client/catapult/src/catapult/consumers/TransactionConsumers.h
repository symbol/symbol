#pragma once
#include "HashCheckOptions.h"
#include "InputUtils.h"
#include "catapult/disruptor/DisruptorConsumer.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/validators/ParallelValidationPolicy.h"

namespace catapult { namespace consumers {

	/// Creates a consumer that calculates hashes of all entities using \a transactionRegistry.
	disruptor::TransactionConsumer CreateTransactionHashCalculatorConsumer(const model::TransactionRegistry& transactionRegistry);

	/// Predicate for determining if a hash is known.
	using KnownHashPredicate = std::function<bool (Timestamp, const Hash256&)>;

	/// Creates a consumer that checks entities for previous processing based on their hash.
	/// \a timeGenerator is used for generating timestamps and \a options specifies additional cache options.
	/// \a knownHashPredicate returns \c true for known hashes.
	disruptor::TransactionConsumer CreateTransactionHashCheckConsumer(
			const std::function<Timestamp ()>& timeGenerator,
			const HashCheckOptions& options,
			const KnownHashPredicate& knownHashPredicate);

	/// Creates a consumer that runs stateless validation using \a pValidator and the specified policy
	/// (\a validationPolicy).
	disruptor::ConstTransactionConsumer CreateTransactionStatelessValidationConsumer(
			const std::shared_ptr<const validators::stateless::AggregateEntityValidator>& pValidator,
			const validators::ParallelValidationPolicyFunc& validationPolicy);

	/// Prototype for a function that is called with new transactions.
	using NewTransactionsSink = std::function<void (TransactionInfos&&)>;

	/// Creates a consumer that calls \a newTransactionsSink with all new transactions.
	/// \note This consumer must be last because it destroys the input.
	disruptor::DisruptorConsumer CreateNewTransactionsConsumer(const NewTransactionsSink& newTransactionsSink);
}}
