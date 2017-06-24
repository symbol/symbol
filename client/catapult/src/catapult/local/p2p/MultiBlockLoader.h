#pragma once
#include "catapult/model/ChainScore.h"
#include "catapult/observers/ObserverTypes.h"
#include <functional>

namespace catapult {
	namespace local { struct LocalNodeStateRef; }
	namespace model {
		struct Block;
		struct BlockChainConfiguration;
	}
}

namespace catapult { namespace local { namespace p2p {

	/// A block dependent entity observer factory.
	using BlockDependentEntityObserverFactory = std::function<const observers::EntityObserver& (const model::Block&)>;

	/// Creates a block dependent entity observer factory that calculates an inflection point from \a lastBlock and \a config.
	/// Prior to the inflection point, \a permanentObserver is returned.
	/// At and after the inflection point, \a transientObserver is returned.
	BlockDependentEntityObserverFactory CreateBlockDependentEntityObserverFactory(
			const model::Block& lastBlock,
			const model::BlockChainConfiguration& config,
			const observers::EntityObserver& transientObserver,
			const observers::EntityObserver& permanentObserver);

	/// Loads a block chain from storage using the supplied observer factory (\a observerFactory) and updating \a stateRef.
	model::ChainScore LoadBlockChain(
			const BlockDependentEntityObserverFactory& observerFactory,
			const LocalNodeStateRef& stateRef);
}}}
