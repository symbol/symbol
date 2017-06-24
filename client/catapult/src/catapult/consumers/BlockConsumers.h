#pragma once
#include "BlockChainProcessor.h"
#include "BlockChainSyncHandlers.h"
#include "HashCheckOptions.h"
#include "InputUtils.h"
#include "catapult/disruptor/DisruptorConsumer.h"
#include "catapult/validators/ParallelValidationPolicy.h"

namespace catapult {
	namespace chain { struct CatapultState; }
	namespace io { class BlockStorageCache; }
	namespace model { class TransactionRegistry; }
}

namespace catapult { namespace consumers {

	/// Creates a consumer that calculates hashes of all entities using \a transactionRegistry.
	disruptor::BlockConsumer CreateBlockHashCalculatorConsumer(const model::TransactionRegistry& transactionRegistry);

	/// Creates a consumer that checks entities for previous processing based on their hash.
	/// \a timeGenerator is used for generating timestamps and \a options specifies additional cache options.
	disruptor::ConstBlockConsumer CreateBlockHashCheckConsumer(
			const std::function<Timestamp ()>& timeGenerator,
			const HashCheckOptions& options);

	/// Creates a consumer that checks a block chain for internal integrity.
	/// A valid chain must have no more than \a maxChainSize blocks.
	disruptor::ConstBlockConsumer CreateBlockChainCheckConsumer(uint32_t maxChainSize);

	/// Predicate for checking whether or not an entity requires validation.
	using RequiresValidationPredicate = model::MatchingEntityPredicate;

	/// Creates a consumer that runs stateless validation using \a pValidator and the specified policy
	/// (\a validationPolicy). Validation will only be performed for entities for which \a requiresValidationPredicate
	/// returns \c true.
	disruptor::ConstBlockConsumer CreateBlockStatelessValidationConsumer(
			const std::shared_ptr<const validators::stateless::AggregateEntityValidator>& pValidator,
			const validators::ParallelValidationPolicyFunc& validationPolicy,
			const RequiresValidationPredicate& requiresValidationPredicate);

	/// Creates a consumer that attempts to synchronize a remote chain with the local chain, which is composed of
	/// state (in \a cache and \a state) and blocks (in \a storage).
	/// \a handlers are used to customize the sync process.
	/// \note This consumer is non-const because it updates the element generation hashes.
	disruptor::DisruptorConsumer CreateBlockChainSyncConsumer(
			cache::CatapultCache& cache,
			state::CatapultState& state,
			io::BlockStorageCache& storage,
			const BlockChainSyncHandlers& handlers);

	/// Prototype for a function that is called with a new block.
	using NewBlockSink = std::function<void (const std::shared_ptr<const model::Block>&)>;

	/// Creates a consumer that calls \a newBlockSink with new blocks that have a source in \a sinkSourceMask.
	/// \note This consumer must be last because it might destroy the input.
	disruptor::DisruptorConsumer CreateNewBlockConsumer(const NewBlockSink& newBlockSink, disruptor::InputSource sinkSourceMask);
}}
