#pragma once
#include "catapult/chain/BatchEntityProcessor.h"
#include "catapult/disruptor/DisruptorElement.h"
#include "catapult/model/WeakEntityInfo.h"
#include <functional>

namespace catapult {
	namespace cache { class ReadOnlyCatapultCache; }
	namespace chain { struct ObserverState; }
}

namespace catapult { namespace consumers {

	/// A tuple composed of a block, a hash and a generation hash.
	class WeakBlockInfo : public model::WeakEntityInfoT<model::Block> {
	public:
			/// Creates a block info.
			constexpr WeakBlockInfo() : m_pGenerationHash(nullptr)
			{}

			/// Creates a block info around \a blockElement.
			constexpr explicit WeakBlockInfo(const model::BlockElement& blockElement)
					: WeakEntityInfoT(blockElement.Block, blockElement.EntityHash)
					, m_pGenerationHash(&blockElement.GenerationHash)
			{}

		public:
			/// The generation hash.
			constexpr const Hash256& generationHash() const {
				return *m_pGenerationHash;
			}

		private:
			const Hash256* m_pGenerationHash;
	};

	/// Function signature for validating, executing and updating a partial block chain given a parent block info
	/// and updating a cache.
	using BlockChainProcessor = std::function<validators::ValidationResult (
			const WeakBlockInfo&,
			disruptor::BlockElements&,
			const observers::ObserverState&)>;

	/// A predicate for determining whether or not two blocks form a hit.
	using BlockHitPredicate = std::function<bool (const model::Block&, const model::Block&, const Hash256&)>;

	/// A factory for creating a predicate for determining whether or not two blocks form a hit.
	using BlockHitPredicateFactory = std::function<BlockHitPredicate (const cache::ReadOnlyCatapultCache&)>;

	/// Creates a block chain processor around the specified block hit predicate factory (\a blockHitPredicateFactory)
	/// and batch entity processor (\a batchEntityProcessor).
	BlockChainProcessor CreateBlockChainProcessor(
			const BlockHitPredicateFactory& blockHitPredicateFactory,
			const chain::BatchEntityProcessor batchEntityProcessor);
}}
